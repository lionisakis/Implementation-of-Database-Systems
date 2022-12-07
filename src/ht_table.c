#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

#define POS_HT_block_info BF_BLOCK_SIZE-sizeof(HT_block_info)-1
#define POS_HT_info BF_BLOCK_SIZE-sizeof(HT_block_info)-sizeof(HT_info)-1


int HT_CreateFile(char *fileName,  int buckets){
  // you create a file with file name
  CALL_OR_DIE(HT_CreateFile(fileName, buckets));
  CALL_OR_DIE(HT_OpenFile(fileName));
  int file_desc;
  // create block info
  HT_block_info blockInfo;
  blockInfo.numOfRecords=0;
  blockInfo.nextBlockNumber=-1;

    // initialize the block
  BF_Block* block;
  BF_Block_Init(&block);


  void* data=BF_Block_GetData(block);

  HT_info* info= malloc(sizeof(*info));
  if(info==NULL){
    printf("Not enough size for malloc\n");
    return NULL;
  }

  info->fileDesc = file_desc;
  info->maxRecordFirstBlock = (BF_BLOCK_SIZE-sizeof(*info)-sizeof(blockInfo)-1)/sizeof(Record);
  info->maxRecordPerBlock=(BF_BLOCK_SIZE-sizeof(blockInfo)-1)/sizeof(Record);
  info->numBuckets= buckets;
  info->hashTable=NULL;

  memcpy(data+POS_HT_block_info,&blockInfo,sizeof(blockInfo));
  memcpy(data+POS_HT_info,info,sizeof(HT_info)); 

  






  CALL_OR_DIE(HT_CloseFile(fileName));
  return 0;
}

HT_info* HT_OpenFile(char *fileName){
  int file_desc;
  // open the file that you created to put HT_info
  CALL_OR_DIE(HT_OpenFile(fileName));


  // find the first block and take the data
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_OR_DIE(BF_GetBlock(file_desc,0,block));
  void* data=BF_Block_GetData(block);
  // take the info data from the block
  HT_info* info=malloc(sizeof(*info));
  if(info==NULL){
    printf("Not enough size for malloc\n");
    return NULL;
  }
  memcpy(info,data+POS_HT_info,sizeof(HT_info));
  // unpin the block and destroy it
  CALL_OR_DIE(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  return info;
  

  

}


int HT_CloseFile( HT_info* HT_info ){
    return 0;
}

int HT_InsertEntry(HT_info* ht_info, Record record){
    return 0;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){
    return 0;
}




