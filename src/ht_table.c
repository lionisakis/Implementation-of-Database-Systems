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

int hashValue(int id,int buckets){
  return id%buckets;
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


int HT_CloseFile( HT_info* HT_info ){
    return 0;
}

int HT_InsertEntry(HT_info* ht_info, Record record){
    return 0;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){
    return 0;
}




