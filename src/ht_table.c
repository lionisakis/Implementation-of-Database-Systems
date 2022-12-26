#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define CALL_HT(call,returnCode)      \
  {                                   \
    BF_ErrorCode code = call;         \
    if (code != BF_OK) {              \
      BF_PrintError(code);            \
      return(returnCode);             \
    }                                 \
  }

#define POS_HT_block_info BF_BLOCK_SIZE-sizeof(HT_block_info)-1
#define POS_HT_info BF_BLOCK_SIZE-sizeof(HT_block_info)-sizeof(HT_info)-1
#define END_BF_BLOCKS -1;

int hashValue(int value,int buckets){
  return value % buckets;
}

// returns the HT_block_info from the block. 
//On error returns NULL
void HT_Get_HT_Block_Info(void* data,HT_block_info* blockInfo){
  // the HT_block_info is always at the last bytes
  memcpy(blockInfo,data+POS_HT_block_info,sizeof(HT_block_info));
}


int HT_CreateFile(char *fileName,  int buckets){
  // you create a file with file name
  CALL_HT(BF_CreateFile(fileName),-1);
  int file_desc;
  CALL_HT(BF_OpenFile(fileName,&file_desc),-1);
  
  // create block info
  HT_block_info blockInfo;
  blockInfo.numOfRecords=0;
  blockInfo.nextBlockNumber=-1;

  // create HT_info 
  HT_info info;
  info.fileDesc = file_desc;
  info.maxRecordFirstBlock = (BF_BLOCK_SIZE-sizeof(HT_info)-sizeof(HT_block_info)-sizeof(int)*buckets-1)/sizeof(Record);
  info.maxRecordPerBlock=(BF_BLOCK_SIZE-sizeof(HT_block_info)-1)/sizeof(Record);
  info.numBuckets= buckets;
  info.posHashTable=POS_HT_info-sizeof(int)*buckets;

  // now go and insert them
  // initialize the block
  BF_Block* block;
  BF_Block_Init(&block);

  // allocate memory and take the data
  CALL_HT(BF_AllocateBlock(file_desc,block),-1);
  void* data=BF_Block_GetData(block);

  // pass blockInfo and info
  memcpy(data+POS_HT_block_info,&blockInfo,sizeof(HT_block_info));
  memcpy(data+POS_HT_info,&info,sizeof(HT_info)); 

  int hashTable[buckets];
  for(int i=0;i<buckets;i++){
    hashTable[i]=-1;
  }

  memcpy(data+info.posHashTable,hashTable,sizeof(int)*buckets);

  BF_Block_SetDirty(block);
  CALL_HT(BF_UnpinBlock(block),-1);
  BF_Block_Destroy(&block);
  CALL_HT(BF_CloseFile(file_desc),-1);
  return 0;
}

HT_info* HT_OpenFile(char *fileName){
  int file_desc;
  // open the file that you created to put HT_info
  CALL_HT(BF_OpenFile(fileName,&file_desc),NULL);

  // find the first block and take the data
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_HT(BF_GetBlock(file_desc,0,block),NULL);
  void* data=BF_Block_GetData(block);
  // take the info data from the block
  HT_info* info=malloc(sizeof(*info));
  if(info==NULL){
    printf("Not enough size for malloc\n");
    return NULL;
  }
  memcpy(info,data+POS_HT_info,sizeof(HT_info));
  // unpin the block and destroy it
  CALL_HT(BF_UnpinBlock(block),NULL);
  BF_Block_Destroy(&block);
  return info;
}


int HT_CloseFile( HT_info* ht_info ){
  // close file
  CALL_HT(BF_CloseFile(ht_info->fileDesc), -1);

  free(ht_info);

  return 0;
}

int HT_InsertEntry(HT_info* ht_info, Record record){
  BF_Block* block;
  BF_Block_Init(&block);
  BF_GetBlock(ht_info->fileDesc,0,block);
  void* data=BF_Block_GetData(block);
  
  //getting the hash table
  int buckets=ht_info->numBuckets;
  int hashTable[buckets];
  memcpy(hashTable,data+ht_info->posHashTable,sizeof(int)*buckets);

  // find the block of the hashTable
  int myBucket= hashValue(record.id,ht_info->numBuckets);
  int currentBlock=hashTable[myBucket];
  int nextBlock=-1;
  
  HT_block_info blockInfo;
  // Check if there is a block
  if(hashTable[myBucket]!=-1){
    if(hashTable[myBucket]!=0){
      BF_UnpinBlock(block);
      BF_GetBlock(ht_info->fileDesc,hashTable[myBucket],block);
      data=BF_Block_GetData(block);
    }
    // while the nextBlock is not -1
    HT_Get_HT_Block_Info(data,&blockInfo);
    int nextBlock=blockInfo.nextBlockNumber;
    while(nextBlock!=-1){
      // unlock the previous block
      BF_UnpinBlock(block);
      // get the next block
      BF_GetBlock(ht_info->fileDesc,nextBlock,block);
      data=BF_Block_GetData(block);
      HT_Get_HT_Block_Info(data,&blockInfo);
      // change the data
      currentBlock=nextBlock;
      nextBlock=blockInfo.nextBlockNumber;
    }
  }

  // if there is at least 1 block
  // then if you have to make a new block
  // it will not be the first block
  int flag=0;
  if(currentBlock==-1){
    for(int i=0;i<buckets;i++){
      if(hashTable[i]!=-1){
        flag=1;
        break;
      }
    }
  }

  // it is the first block of the bucket that we will make 
  if(currentBlock==-1){
    // there is room in the first block that no other bucket has taken
    if(flag==0){
      // copy the new block to the hashTable
      hashTable[myBucket]=0;
      memcpy(data+ht_info->posHashTable,hashTable,sizeof(int)*ht_info->numBuckets);
  
      // pass the new HT_block_info 
      HT_block_info blockInfo;
      void* data=BF_Block_GetData(block);
      HT_Get_HT_Block_Info(data,&blockInfo);
      blockInfo.numOfRecords++;
      memcpy(data+POS_HT_block_info,&blockInfo,sizeof(HT_block_info));

      // pass the record
      memcpy(data,&record,sizeof(Record));

      // say that the first block is changed and unpin it
      BF_Block_SetDirty(block);
      CALL_HT(BF_UnpinBlock(block),-1);

      // destroy the block
      BF_Block_Destroy(&block);
      return 0;
    }
    else{
      // copy the new block to the hashTable
      CALL_HT(BF_GetBlockCounter(ht_info->fileDesc,&hashTable[myBucket]),-1);
      memcpy(data+ht_info->posHashTable,hashTable,sizeof(int)*ht_info->numBuckets);

      // say that the first block is changed and unpin it
      BF_Block_SetDirty(block);
      CALL_HT(BF_UnpinBlock(block),-1);

      // allocate a new block and get the data
      CALL_HT(BF_AllocateBlock(ht_info->fileDesc,block),-1);
      void* data=BF_Block_GetData(block);

      // form the new blockInfo
      HT_block_info blockInfo;
      blockInfo.nextBlockNumber=-1;
      blockInfo.numOfRecords=1;

      // copy all the data to the block
      memcpy(data+POS_HT_block_info,&blockInfo,sizeof(HT_block_info));
      memcpy(data,&record,sizeof(Record));

      // say that the first block is changed and unpin it
      // destroy the block
      BF_Block_SetDirty(block);
      CALL_HT(BF_UnpinBlock(block),-1);
      BF_Block_Destroy(&block);
      return hashTable[myBucket];
    }
  }
  else{
    // check if the record that you are going to insert is full or not
    int insert=1;
    if(currentBlock==0 && blockInfo.numOfRecords<ht_info->maxRecordFirstBlock)
      insert=0;
    else if(currentBlock>0 && blockInfo.numOfRecords<ht_info->maxRecordPerBlock)
      insert=0;


    // it is full so insert new block
    if(insert){
      // copy the new block to the hashTable
      CALL_HT(BF_GetBlockCounter(ht_info->fileDesc,&blockInfo.nextBlockNumber),-1);
      int next=blockInfo.nextBlockNumber;
      memcpy(data+POS_HT_block_info,&blockInfo,sizeof(HT_block_info));

      // say that the first block is changed and unpin it
      BF_Block_SetDirty(block);
      CALL_HT(BF_UnpinBlock(block),-1);

      // allocate a new block and get the data
      CALL_HT(BF_AllocateBlock(ht_info->fileDesc,block),-1);
      void* data=BF_Block_GetData(block);

      memcpy(data,&record,sizeof(Record));

      // form the new blockInfo
      HT_block_info blockInfo;
      blockInfo.nextBlockNumber=-1;
      blockInfo.numOfRecords=1;

      // copy all the data to the block
      memcpy(data+POS_HT_block_info,&blockInfo,sizeof(HT_block_info));

      // say that the first block is changed and unpin it
      // destroy the block
      BF_Block_SetDirty(block);
      CALL_HT(BF_UnpinBlock(block),-1);
      BF_Block_Destroy(&block);
      return next;
    }
    else{
      memcpy(data+blockInfo.numOfRecords*sizeof(Record),&record,sizeof(Record));

      // insert the record to the correct position
      Record* record=(Record*)(data+(blockInfo.numOfRecords-1)*sizeof(Record));
      record=(Record*)(data+blockInfo.numOfRecords*sizeof(Record));
      
      // change HT_block_info to data
      blockInfo.numOfRecords++;
      memcpy(data+POS_HT_block_info,&blockInfo,sizeof(HT_block_info));

      // say that the first block is changed and unpin it
      // destroy the block
      BF_Block_SetDirty(block);
      CALL_HT(BF_UnpinBlock(block),-1);
      BF_Block_Destroy(&block);
      return currentBlock;
    }
  }
  return -1;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){

  int numberOfVisitedBlocks=0;
  int file_desc = ht_info->fileDesc;
  int buckets = ht_info->numBuckets;

  //find first block to get hash table
  BF_Block* block;
  BF_Block_Init(&block);
  CALL_HT(BF_GetBlock(file_desc, 0, block), -1);

  //find ht_block_info of first block
  void* data = BF_Block_GetData(block); 
  HT_block_info blockInfo;
  HT_Get_HT_Block_Info(data,&blockInfo);

  //getting the hash table
  int hashtable[buckets];
  memcpy(hashtable,data+ht_info->posHashTable,sizeof(int)*buckets);
  
  //in which bucket should I search?
  int myBucket= hashValue(*(int*)value,ht_info->numBuckets);

  int blockId=hashtable[myBucket];
  if(blockId==-1)
    return 0;
  //find my id block
  BF_Block* myBlock;
  BF_Block_Init(&myBlock);
  CALL_HT(BF_GetBlock(file_desc, blockId, myBlock), -1);

  //find data & ht_block_info of my id block
  void* myData = BF_Block_GetData(myBlock); 
  HT_block_info myBlockInfo;
  HT_Get_HT_Block_Info(myData,&myBlockInfo);

  int blockRecords = myBlockInfo.numOfRecords;
  //flag: has this bucket any other block? yes:0, no:-1
  int flag=0;

  while(flag!=-1) {
    numberOfVisitedBlocks++;
    Record* rec = (Record*)myData;
    // checking the block's records
    for(int i=0; i<blockRecords; i++) {
      if(rec[i].id == *(int *)value)
        printRecord(rec[i]);
    }
    //find next block to check
    if (myBlockInfo.nextBlockNumber!=-1){
      int next = myBlockInfo.nextBlockNumber;
      CALL_HT(BF_UnpinBlock(myBlock),-1);
      CALL_HT(BF_GetBlock(ht_info->fileDesc,next,myBlock),-1);
      myData = BF_Block_GetData(myBlock); 
      
      //find next block's hp_block_info
      HT_Get_HT_Block_Info(myData,&myBlockInfo);
      blockRecords=myBlockInfo.numOfRecords;
    }
    else 
      flag=-1;
  }

  // set it so anyone can take it
  CALL_HT(BF_UnpinBlock(block),-1);
  CALL_HT(BF_UnpinBlock(myBlock),-1);

  BF_Block_Destroy(&block);
  BF_Block_Destroy(&myBlock);
  return numberOfVisitedBlocks;
}


