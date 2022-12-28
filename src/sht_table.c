#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "sht_table.h"
#include "ht_table.h"
#include "record.h"

#define CALL_SHT(call,returnCode)     \
  {                                   \
    BF_ErrorCode code = call;         \
    if (code != BF_OK) {              \
      BF_PrintError(code);            \
      return(returnCode);             \
    }                                 \
  }

#define POS_SHT_block_info BF_BLOCK_SIZE-sizeof(SHT_block_info)-1
#define POS_SHT_info BF_BLOCK_SIZE-sizeof(SHT_block_info)-sizeof(SHT_info)-1

// returns the SHT_block_info from the block. 
//On error returns NULL
void SHT_Get_SHT_Block_Info(void* data,SHT_block_info* blockInfo){
  // the SHT_block_info is always at the last bytes
  memcpy(blockInfo,data+POS_SHT_block_info,sizeof(SHT_block_info));
}


int SHT_hashValue(char* string,int buckets){
  int sum=0;
  while(*string!='\0'){
    sum+=*string-'a';
    string++;
  }
  return sum%buckets;
}

int SHT_CreateSecondaryIndex(char *sfileName,  int buckets, char* fileName){
  // you create a file with file name
  CALL_SHT(BF_CreateFile(sfileName),-1);
  int file_desc;
  CALL_SHT(BF_OpenFile(sfileName,&file_desc),-1);
  // create block info
  SHT_block_info blockInfo;
  blockInfo.numOfInfo=0;
  blockInfo.nextBlockNumber=-1;

  // create HT_info 
  SHT_info info;
  info.fileDesc = -1;
  info.maxInfoFirstBlock = (BF_BLOCK_SIZE-sizeof(SHT_info)-sizeof(SHT_block_info)-sizeof(int)*buckets-1)/sizeof(SHT_node_info);
  printf("%d\n",info.maxInfoFirstBlock);
  info.maxInfoPerBlock=(BF_BLOCK_SIZE-sizeof(SHT_block_info)-1)/sizeof(SHT_node_info);
  printf("%d\n",info.maxInfoPerBlock);
  info.numBuckets= buckets;
  info.posHashTable=POS_SHT_info-sizeof(int)*buckets;

  // now go and insert them
  // initialize the block
  BF_Block* block;
  BF_Block_Init(&block);

  // allocate memory and take the data
  CALL_SHT(BF_AllocateBlock(file_desc,block),-1);
  void* data=BF_Block_GetData(block);

  // pass blockInfo and info
  memcpy(data+POS_SHT_block_info,&blockInfo,sizeof(SHT_block_info));
  memcpy(data+POS_SHT_info,&info,sizeof(SHT_info)); 

  int hashTable[buckets];
  for(int i=0;i<buckets;i++){
    hashTable[i]=-1;
  }

  memcpy(data+info.posHashTable,hashTable,sizeof(int)*buckets);

  BF_Block_SetDirty(block);
  CALL_SHT(BF_UnpinBlock(block),-1);
  BF_Block_Destroy(&block);
  CALL_SHT(BF_CloseFile(file_desc),-1);
  return 0;
}

SHT_info* SHT_OpenSecondaryIndex(char *indexName){
  int file_desc;
  // open the file that you created to put HT_info
  CALL_SHT(BF_OpenFile(indexName,&file_desc),NULL);

  // find the first block and take the data
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_SHT(BF_GetBlock(file_desc,0,block),NULL);
  void* data=BF_Block_GetData(block);
  // take the info data from the block
  SHT_info* info=malloc(sizeof(*info));
  if(info==NULL){
    printf("Not enough size for malloc\n");
    return NULL;
  }
  memcpy(info,data+POS_SHT_info,sizeof(SHT_info));
  info->fileDesc=file_desc;
  memcpy(data+POS_SHT_info,info,sizeof(SHT_info));
  // unpin the block and destroy it
  CALL_SHT(BF_UnpinBlock(block),NULL);
  BF_Block_Destroy(&block);
  return info;
}


int SHT_CloseSecondaryIndex( SHT_info* SHT_info ){
  // close file
  CALL_SHT(BF_CloseFile(SHT_info->fileDesc), -1);

  free(SHT_info);

  return 0;
}

int SHT_SecondaryInsertEntry(SHT_info* sht_info, Record record, int block_id){
  BF_Block* block;
  BF_Block_Init(&block);
  BF_GetBlock(sht_info->fileDesc,0,block);
  void* data=BF_Block_GetData(block);
  
  //getting the hash table
  int buckets=sht_info->numBuckets;
  int hashTable[buckets];
  memcpy(hashTable,data+sht_info->posHashTable,sizeof(int)*buckets);

  // find the block of the hashTable
  int myBucket= SHT_hashValue(record.name,sht_info->numBuckets);
  int currentBlock=hashTable[myBucket];
  int nextBlock=-1;
  
  SHT_block_info blockInfo;
  // Check if there is a block
  if(hashTable[myBucket]!=-1){
    if(hashTable[myBucket]!=0){
      BF_UnpinBlock(block);
      BF_GetBlock(sht_info->fileDesc,hashTable[myBucket],block);
      data=BF_Block_GetData(block);
    }
    // while the nextBlock is not -1
    memcpy(&blockInfo,data+POS_SHT_block_info,sizeof(SHT_block_info));
    nextBlock=blockInfo.nextBlockNumber;
    
    while(nextBlock!=-1){
      // unlock the previous block
      BF_UnpinBlock(block);
      // get the next block
      BF_GetBlock(sht_info->fileDesc,nextBlock,block);
      data=BF_Block_GetData(block);
      memcpy(&blockInfo,data+POS_SHT_block_info,sizeof(SHT_block_info));

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
      memcpy(data+sht_info->posHashTable,hashTable,sizeof(int)*sht_info->numBuckets);
  
      // pass the new HT_block_info 
      SHT_block_info blockInfo2;
      void* data2=BF_Block_GetData(block);
      SHT_Get_SHT_Block_Info(data,&blockInfo2);
      blockInfo2.numOfInfo++;
      memcpy(data2+POS_SHT_block_info,&blockInfo2,sizeof(SHT_block_info));

      // pass the SHT_node_info
      SHT_node_info nodeInfo;
      nodeInfo.blockID=block_id;
      strcpy(nodeInfo.name,record.name);
      memcpy(data2,&nodeInfo,sizeof(SHT_node_info));

      // say that the first block is changed and unpin it
      BF_Block_SetDirty(block);
      CALL_SHT(BF_UnpinBlock(block),-1);

      // destroy the block
      BF_Block_Destroy(&block);
      return 0;
    }
    else{
      // copy the new block to the hashTable
      CALL_SHT(BF_GetBlockCounter(sht_info->fileDesc,&hashTable[myBucket]),-1);
      memcpy(data+sht_info->posHashTable,hashTable,sizeof(int)*sht_info->numBuckets);

      // say that the first block is changed and unpin it
      BF_Block_SetDirty(block);
      CALL_SHT(BF_UnpinBlock(block),-1);

      // allocate a new block and get the data
      CALL_SHT(BF_AllocateBlock(sht_info->fileDesc,block),-1);
      void* data2=BF_Block_GetData(block);

      // form the new blockInfo
      SHT_block_info blockInfo2;
      blockInfo2.nextBlockNumber=-1;
      blockInfo2.numOfInfo=1;

      // copy all the data to the block
      memcpy(data2+POS_SHT_block_info,&blockInfo2,sizeof(SHT_block_info));
      // pass the SHT_node_info
      SHT_node_info nodeInfo;
      nodeInfo.blockID=block_id;
      strcpy(nodeInfo.name,record.name);
      memcpy(data2,&nodeInfo,sizeof(SHT_node_info));

      // say that the first block is changed and unpin it
      // destroy the block
      BF_Block_SetDirty(block);
      CALL_SHT(BF_UnpinBlock(block),-1);
      BF_Block_Destroy(&block);
      return hashTable[myBucket];
    }
  }
  else{
    // check if the record that you are going to insert is full or not
    int insert=1;
    if(currentBlock==0 && blockInfo.numOfInfo<sht_info->maxInfoFirstBlock)
      insert=0;
    else if(currentBlock>0 && blockInfo.numOfInfo<sht_info->maxInfoPerBlock-1)
      insert=0;

    // it is full so insert new block
    if(insert){
      // copy the new block to the hashTable
      int size;
      CALL_SHT(BF_GetBlockCounter(sht_info->fileDesc,&size),-1);
      blockInfo.nextBlockNumber=size;
      memcpy(data+POS_SHT_block_info,&blockInfo,sizeof(SHT_block_info));

      // say that the first block is changed and unpin it
      BF_Block_SetDirty(block);
      CALL_SHT(BF_UnpinBlock(block),-1);

      // allocate a new block and get the data
      CALL_SHT(BF_AllocateBlock(sht_info->fileDesc,block),-1);
      void* data2=BF_Block_GetData(block);

      // pass the SHT_node_info
      SHT_node_info nodeInfo;
      nodeInfo.blockID=block_id;
      strcpy(nodeInfo.name,record.name);
      memcpy(data2,&nodeInfo,sizeof(SHT_node_info));

      // form the new blockInfo
      SHT_block_info blockInfo2;
      blockInfo2.numOfInfo=1;
      blockInfo2.nextBlockNumber=-1;

      // copy all the data to the block
      memcpy(data2+POS_SHT_block_info,&blockInfo2,sizeof(SHT_block_info));

      // say that the first block is changed and unpin it
      // destroy the block
      BF_Block_SetDirty(block);
      CALL_SHT(BF_UnpinBlock(block),-1);
      BF_Block_Destroy(&block);
      return size;
    }
    else{
      // pass the SHT_node_info
      SHT_node_info nodeInfo;
      nodeInfo.blockID=block_id;
      strcpy(nodeInfo.name,record.name);
      memcpy(data+blockInfo.numOfInfo*sizeof(SHT_node_info),&nodeInfo,sizeof(SHT_node_info));

      // change HT_block_info to data
      blockInfo.numOfInfo++;
      memcpy(data+POS_SHT_block_info,&blockInfo,sizeof(SHT_block_info));

      // say that the first block is changed and unpin it
      // destroy the block
      BF_Block_SetDirty(block);
      CALL_SHT(BF_UnpinBlock(block),-1);
      BF_Block_Destroy(&block);
      return currentBlock;
    }
  }
  return -1;
}

int SHT_SecondaryGetAllEntries(HT_info* ht_info, SHT_info* sht_info, char* name){
  // ~~ HT PART ~~//

  int ht_file_desc = ht_info->fileDesc;
  int ht_buckets = ht_info->numBuckets;

  //find first block to get hash table
  BF_Block* ht_block;
  BF_Block_Init(&ht_block);
  CALL_HT(BF_GetBlock(ht_file_desc, 0, ht_block), -1);

  //find ht_block_info of first block
  void* ht_data = BF_Block_GetData(ht_block); 
  HT_block_info ht_blockInfo;
  HT_Get_HT_Block_Info(ht_data,&ht_blockInfo);

  //getting the hash table
  int ht_hashtable[ht_buckets];
  memcpy(ht_hashtable,ht_data+ht_info->posHashTable,sizeof(int)*ht_buckets);
  
    // ~~END OF HT PART ~~//
  
  int numberOfVisitedBlocks=0;
  int file_desc = sht_info->fileDesc;
  int buckets = sht_info->numBuckets;

  //find first block to get hash table
  BF_Block* block;
  BF_Block_Init(&block);
  CALL_SHT(BF_GetBlock(file_desc, 0, block), -1);

  //find sht_block_info of first block
  void* data = BF_Block_GetData(block); 
  SHT_block_info blockInfo;
  SHT_Get_SHT_Block_Info(data,&blockInfo);

  //getting the hash table
  int hashtable[buckets];
  memcpy(hashtable,data+ht_info->posHashTable,sizeof(int)*buckets);

  //in which bucket should I search?
  int myBucket= SHT_hashValue(name, sht_info->numBuckets);

  int blockId=hashtable[myBucket];
  if(blockId==-1)
    return 0;
  //find my id block
  BF_Block* myBlock;
  BF_Block_Init(&myBlock);
  CALL_SHT(BF_GetBlock(file_desc, blockId, myBlock), -1);

  //find data & sht_block_info of my id block
  void* myData = BF_Block_GetData(myBlock); 
  SHT_block_info myBlockInfo;
  SHT_Get_SHT_Block_Info(myData,&myBlockInfo);

  int numOfInfo = myBlockInfo.numOfInfo;
  //flag: has this bucket any other block? yes:0, no:-1
  int flag=0;

  while(flag!=-1) {
    numberOfVisitedBlocks++;
    SHT_node_info* info = (SHT_node_info*)myData;
    // checking the block's records
    for(int i=0; i<numOfInfo; i++) {
      if(strcmp(name,info->name)==0) {
        //find my id block
        BF_Block* recBlock;
        BF_Block_Init(&recBlock);
        CALL_HT(BF_GetBlock(file_desc, info->blockID, recBlock), -1);

        //find data & ht_block_info of my id block
        void* recData = BF_Block_GetData(recBlock); 
        HT_block_info recBlockInfo;
        HT_Get_HT_Block_Info(recData,&recBlockInfo);

        int recBlockRecords = recBlockInfo.numOfRecords;
        //flag: has this bucket any other block? yes:0, no:-1
        int flag2=0;

        while(flag2!=-1) {
            numberOfVisitedBlocks++;
            Record* rec = (Record*)recData;
            // checking the block's records
            for(int i=0; i<recBlockRecords; i++) {
              if(strcmp(name,rec[i].name)==0)
                printRecord(rec[i]);
              }
            //find next block to check
            if (recBlockInfo.nextBlockNumber!=-1){
              int next = recBlockInfo.nextBlockNumber;
              CALL_HT(BF_UnpinBlock(recBlock),-1);
              CALL_HT(BF_GetBlock(ht_info->fileDesc,next,recBlock),-1);
              myData = BF_Block_GetData(recBlock); 
      
              //find next block's hp_block_info
              HT_Get_HT_Block_Info(recData,&recBlockInfo);
              recBlockRecords=recBlockInfo.numOfRecords;
            }
            else 
              flag2=-1;
      }
  
        
        }
    }
    //find next block to check
    if (myBlockInfo.nextBlockNumber!=-1){
      int next = myBlockInfo.nextBlockNumber;
      CALL_SHT(BF_UnpinBlock(myBlock),-1);
      CALL_SHT(BF_GetBlock(ht_info->fileDesc,next,myBlock),-1);
      myData = BF_Block_GetData(myBlock); 
      
      //find next block's hp_block_info
      HT_Get_HT_Block_Info(myData,&myBlockInfo);
      
    }
    else 
      flag=-1;
  }







  // set it so anyone can take it
  CALL_HT(BF_UnpinBlock(ht_block),-1);
  CALL_SHT(BF_UnpinBlock(block),-1);
  CALL_SHT(BF_UnpinBlock(myBlock),-1);

  BF_Block_Destroy(&ht_block);
  BF_Block_Destroy(&block);
  BF_Block_Destroy(&myBlock);

  return numberOfVisitedBlocks;
}


