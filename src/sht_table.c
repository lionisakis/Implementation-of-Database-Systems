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
  printf("Bucket[%d]: %d\n",myBucket,hashTable[myBucket]);
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
    int nextBlock=blockInfo.nextBlockNumber;
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
      SHT_block_info blockInfo;
      void* data=BF_Block_GetData(block);
      memcpy(&blockInfo,data+POS_SHT_block_info,sizeof(SHT_block_info));
      blockInfo.numOfInfo++;
      memcpy(data+POS_SHT_block_info,&blockInfo,sizeof(SHT_block_info));

      // pass the record
      SHT_node_info nodeInfo;
      strcpy(nodeInfo.name,record.name);
      nodeInfo.blockID=block_id;
      memcpy(data,&nodeInfo,sizeof(SHT_node_info));

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
      void* data=BF_Block_GetData(block);

      // form the new blockInfo
      SHT_block_info blockInfo;
      blockInfo.nextBlockNumber=-1;
      blockInfo.numOfInfo=1;

      // copy all the data to the block
      memcpy(data+POS_SHT_block_info,&blockInfo,sizeof(SHT_block_info));
      
      // pass the record
      SHT_node_info nodeInfo;
      strcpy(nodeInfo.name,record.name);
      nodeInfo.blockID=block_id;
      memcpy(data,&nodeInfo,sizeof(SHT_node_info));

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
    printf("blockInfo.numOfInfo %d sht_info->maxInfoFirstBlock %d\n",blockInfo.numOfInfo,sht_info->maxInfoFirstBlock);
    if(currentBlock==0 && blockInfo.numOfInfo<sht_info->maxInfoFirstBlock)
      insert=0;
    else if(currentBlock>0 && blockInfo.numOfInfo<sht_info->maxInfoPerBlock)
      insert=0;

    printf("insert: %d\n",insert);
    // it is full so insert new block
    if(insert){
      // copy the new block to the hashTable
      CALL_SHT(BF_GetBlockCounter(sht_info->fileDesc,&blockInfo.nextBlockNumber),-1);
      int next=blockInfo.nextBlockNumber;
      memcpy(data+POS_SHT_block_info,&blockInfo,sizeof(SHT_block_info));

      // say that the first block is changed and unpin it
      BF_Block_SetDirty(block);
      CALL_SHT(BF_UnpinBlock(block),-1);

      // allocate a new block and get the data
      CALL_SHT(BF_AllocateBlock(sht_info->fileDesc,block),-1);
      void* data=BF_Block_GetData(block);

      // pass the record
      SHT_node_info nodeInfo;
      strcpy(nodeInfo.name,record.name);
      nodeInfo.blockID=block_id;
      memcpy(data,&nodeInfo,sizeof(SHT_node_info));

      // form the new blockInfo
      HT_block_info blockInfo;
      blockInfo.nextBlockNumber=-1;
      blockInfo.numOfRecords=1;

      // copy all the data to the block
      memcpy(data+POS_SHT_block_info,&blockInfo,sizeof(SHT_block_info));

      // say that the first block is changed and unpin it
      // destroy the block
      BF_Block_SetDirty(block);
      CALL_SHT(BF_UnpinBlock(block),-1);
      BF_Block_Destroy(&block);
      return next;
    }
    else{
      // pass the record
      printf("1\n");
      SHT_node_info nodeInfo;
      printf("2\n");
      strcpy(nodeInfo.name,record.name);
      printf("3\n");
      nodeInfo.blockID=block_id;
      printf("4\n");
      memcpy(data+blockInfo.numOfInfo*sizeof(Record),&nodeInfo,sizeof(SHT_node_info));
      printf("5\n");

      // change HT_block_info to data
      blockInfo.numOfInfo++;
      printf("6\n");
      memcpy(data+POS_SHT_block_info,&blockInfo,sizeof(SHT_block_info));
      printf("7\n");

      // say that the first block is changed and unpin it
      // destroy the block
      BF_Block_SetDirty(block);
      printf("8\n");
      CALL_SHT(BF_UnpinBlock(block),-1);
      printf("9\n");
      BF_Block_Destroy(&block);
      printf("10\n");
      return currentBlock;
    }
  }
  return -1;
}

int SHT_SecondaryGetAllEntries(HT_info* ht_info, SHT_info* sht_info, char* name){

}


