#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"

#define RECORDS_NUM 200 // you can change it if you want
#define FILE_NAME "data.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int HashStatistics(char* fileName);

int main() {
  BF_Init(LRU);

  HT_CreateFile(FILE_NAME,10);
  HT_info* info = HT_OpenFile(FILE_NAME);

  Record record;
  srand(12569874);
  int r;
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    HT_InsertEntry(info, record);
  }

  int id = rand() % RECORDS_NUM;
  HT_GetAllEntries(info, &id);

  HashStatistics(FILE_NAME);

  HT_CloseFile(info);
  BF_Close();
}


int HashStatistics(char* fileName) {
  printf("FILE STATISTICS\n\n");
  HT_info* ht_info = HT_OpenFile(fileName);
  int file_desc = ht_info->fileDesc;
  long int buckets = ht_info->numBuckets;

  //Initialization
  long int recordsPerBucket[buckets][3];
  long int overflowBlocksPerBucket[buckets];
  long int overflowBuckets=0;
  long int fileBlocks=0;

  for(int i=0; i<buckets; i++)
    overflowBlocksPerBucket[i]=0;
  
  //find first block to get hash table
  BF_Block* firstBlock;
  BF_Block_Init(&firstBlock);
  CALL_OR_DIE(BF_GetBlock(file_desc, 0, firstBlock));

  //find ht_block_info of first block
  void* fistData = BF_Block_GetData(firstBlock); 
  HT_block_info firstBlockInfo;
  HT_Get_HT_Block_Info(fistData,&firstBlockInfo);

  //getting the hash table
  int hashtable[buckets];
  memcpy(hashtable,fistData+ht_info->posHashTable,sizeof(int)*buckets);

  
  int max=0;
  int min=0;
  int sum=0;
  for(int i=0; i<buckets; i++) {

    //find current block
    BF_Block* block;
    BF_Block_Init(&block);
    int blockId=hashtable[i];
    if(blockId==-1)
      continue;
    CALL_OR_DIE(BF_GetBlock(file_desc, blockId, block));

    //find data & ht_block_info
    void* data = BF_Block_GetData(block); 
    HT_block_info blockInfo;
    HT_Get_HT_Block_Info(data,&blockInfo);
    int blockRecords = blockInfo.numOfRecords;
    min=blockRecords;
    int flag=0;
    
    long int bucketBlocks=0;
    while(flag!=-1) {
      fileBlocks++;
      bucketBlocks++;
      if (blockRecords>max) {
        max = blockRecords;
      }
      if(blockRecords<min) {
        min = blockRecords;
      }
      sum += blockRecords;
      //find next block to check
      if (blockInfo.nextBlockNumber!=-1){
        overflowBlocksPerBucket[i]++;
        int next = blockInfo.nextBlockNumber;
        CALL_OR_DIE(BF_UnpinBlock(block));
        CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc,next,block));
        data = BF_Block_GetData(block); 
      
        //find next block's hp_block_info
        HT_Get_HT_Block_Info(data,&blockInfo);
        blockRecords=blockInfo.numOfRecords;
      }
      else 
        flag=-1;
    }

    recordsPerBucket[i][0] = max;
    recordsPerBucket[i][1] = sum/bucketBlocks;
    recordsPerBucket[i][2]= min;
    if(overflowBlocksPerBucket[i]>0)
      overflowBuckets++;
  }

  long int averageBlocksPerBucket = fileBlocks/buckets;

  printf("This file has %ld blocks\n", fileBlocks);

  for(int i=0; i<buckets; i++) {
    printf ("Bucket %d Max Records: %ld\n",i, recordsPerBucket[i][0]);
    printf ("Bucket %d Average Records: %ld\n", i, recordsPerBucket[i][1]);
    printf ("Bucket %d Min Records: %ld\n", i, recordsPerBucket[i][2]);
  }

  printf("Average blocks per bucket: %ld\n", averageBlocksPerBucket);

  printf("Buckets with overflow blocks: %ld\n", overflowBuckets);

  for(int i=0; i<buckets; i++) {
    if(overflowBlocksPerBucket[i]>0) {
      printf("Bucket %d Overflow Blocks: %ld\n",i, overflowBlocksPerBucket[i]);
    }
  }
}
