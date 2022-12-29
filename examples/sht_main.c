#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "sht_table.h"

#define RECORDS_NUM 200 // you can change it if you want
#define FILE_NAME "data.db"
#define INDEX_NAME "index.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int HashStatistics(char* fileName, char *indexName);

int main() {
    srand(12569874);
    BF_Init(LRU);
    // Αρχικοποιήσεις
    HT_CreateFile(FILE_NAME,10);
    SHT_CreateSecondaryIndex(INDEX_NAME,10,FILE_NAME);

    HT_info* info = HT_OpenFile(FILE_NAME);
    SHT_info* index_info = SHT_OpenSecondaryIndex(INDEX_NAME);

    // Θα ψάξουμε στην συνέχεια το όνομα searchName
    Record record=randomRecord();
    char searchName[15];
    strcpy(searchName, record.name);

    // Κάνουμε εισαγωγή τυχαίων εγγραφών τόσο στο αρχείο κατακερματισμού τις οποίες προσθέτουμε και στο δευτερεύον ευρετήριο
    printf("Insert Entries\n");
    for (int id = 0; id < RECORDS_NUM; ++id) {
        record = randomRecord();
        int block_id = HT_InsertEntry(info, record);
        SHT_SecondaryInsertEntry(index_info, record, block_id);
    }

    // // Τυπώνουμε όλες τις εγγραφές με όνομα searchName
    printf("RUN PrintAllEntries for name %s\n",searchName);
    SHT_SecondaryGetAllEntries(info,index_info,searchName);

    // Κλείνουμε το αρχείο κατακερματισμού και το δευτερεύον ευρετήριο
    SHT_CloseSecondaryIndex(index_info);
    HT_CloseFile(info);
    // 

    HashStatistics(FILE_NAME, INDEX_NAME);

    BF_Close();
}

int HashStatistics(char* fileName, char *indexName) {

  printf("FILE STATISTICS-SHT\n\n");
  HT_info* ht_info = HT_OpenFile(fileName);
  SHT_info* sht_info = SHT_OpenSecondaryIndex(indexName);
  int file_desc = sht_info->fileDesc;
  long int buckets = sht_info->numBuckets;

  //Initialization
  long int infoPerBucket[buckets][3];
  long int overflowBlocksPerBucket[buckets];
  long int overflowBuckets=0;
  long int fileBlocks=0;

  for(int i=0; i<buckets; i++)
    overflowBlocksPerBucket[i]=0;

  
  //find first block to get hash table
  BF_Block* firstBlock;
  BF_Block_Init(&firstBlock);
  CALL_OR_DIE(BF_GetBlock(file_desc, 0, firstBlock));

  //find sht_block_info of first block
  void* fistData = BF_Block_GetData(firstBlock); 
  SHT_block_info firstBlockInfo;
  SHT_Get_SHT_Block_Info(fistData,&firstBlockInfo);
  
  CALL_OR_DIE(BF_UnpinBlock(firstBlock));
  BF_Block_Destroy(&firstBlock);
  
  //getting the hash table
  int hashtable[buckets];
  memcpy(hashtable,fistData+sht_info->posHashTable,sizeof(int)*buckets);

  for(int i=0; i<buckets; i++) {

    int max=0;
    int min=0;
    int sum=0;

    long int bucketBlocks=0;
    int sht_blockId=hashtable[i];
    if(sht_blockId==-1) {
      infoPerBucket[i][0] = 0;
      infoPerBucket[i][1] = 0;
      infoPerBucket[i][2] = 0;
      continue;
    }
      
    //find current block
    BF_Block* block;
    BF_Block_Init(&block);
    CALL_OR_DIE(BF_GetBlock(file_desc, sht_blockId, block));

    //find data & ht_block_info
    void* data = BF_Block_GetData(block); 
    SHT_block_info blockInfo;
    SHT_Get_SHT_Block_Info(data,&blockInfo);
    int numOfInfo = blockInfo.numOfInfo;
 
    min=numOfInfo;
    int flag=0;

    
    while(flag!=-1) {
      fileBlocks++;
      bucketBlocks++;
      if (numOfInfo>max) {
        max = numOfInfo;
      }
      if(numOfInfo<min) {
        min = numOfInfo;
      }
      sum += numOfInfo;

      //find next block to check
    if (blockInfo.nextBlockNumber!=-1){
      overflowBlocksPerBucket[i]++;
      int next = blockInfo.nextBlockNumber;
      CALL_OR_DIE(BF_UnpinBlock(block));
      CALL_OR_DIE(BF_GetBlock(sht_info->fileDesc,next,block));
      data = BF_Block_GetData(block); 
      
      //find next block's hp_block_info
      SHT_Get_SHT_Block_Info(data,&blockInfo);
      numOfInfo=blockInfo.numOfInfo;
    }
    else 
        flag=-1;
    }

    infoPerBucket[i][0] = max;
    infoPerBucket[i][1] = sum/bucketBlocks;
    infoPerBucket[i][2]= min;
    if(overflowBlocksPerBucket[i]>0)
      overflowBuckets++;
    
    CALL_OR_DIE(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
  }
  double averageBlocksPerBucket = (double)fileBlocks/(double)buckets;

  printf("This file has %ld blocks\n", fileBlocks);

  for(int i=0; i<buckets; i++) {
    printf ("Bucket %d Max INFO: %ld\n",i, infoPerBucket[i][0]);
    printf ("Bucket %d Average INFO: %ld\n", i, infoPerBucket[i][1]);
    printf ("Bucket %d Min INFO: %ld\n", i, infoPerBucket[i][2]);
    printf(" ~~~ \n");
  }

  printf("Average blocks per bucket: %f\n", averageBlocksPerBucket);

  printf("Buckets with overflow blocks: %ld\n", overflowBuckets);

  for(int i=0; i<buckets; i++) {
    if(overflowBlocksPerBucket[i]>0) {
      printf("Bucket %d Overflow Blocks: %ld\n",i, overflowBlocksPerBucket[i]);
    }
  }
    
    SHT_CloseSecondaryIndex(sht_info);
    HT_CloseFile(ht_info);
    return 0;

}