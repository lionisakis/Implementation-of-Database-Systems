#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call,returnCode)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return returnCode;        \
  }                         \
}

int HP_CreateFile(char *fileName){
  BF_ErrorCode err;
  // you create a file with file name
  CALL_BF(BF_CreateFile(fileName),-1);
  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  BF_ErrorCode err;
  int file_desc;
  // open the file that you created to put HP_info
  CALL_BF(BF_OpenFile(fileName,&file_desc),NULL);

  // create block info
  HP_block_info blockInfo;
  blockInfo.nextBlock=NULL;
  blockInfo.numOfRecords=0;

  // create the info to store
  HP_info* info= malloc(sizeof(*info));
  if(info==NULL)
    return NULL;

  info->fileDesc = file_desc;
  info->maxRecordFirstBlock = (sizeof(Record)-sizeof(info)-sizeof(blockInfo))/BF_BLOCK_SIZE;
  info->maxRecordPerBlock = (sizeof(Record)-sizeof(blockInfo))/BF_BLOCK_SIZE;

  // initialize the block
  BF_Block* block;
  BF_Block_Init(&block);

  // make a block to store the data 
  CALL_BF(BF_AllocateBlock(file_desc,block),NULL);
  void* data=BF_Block_GetData(block);

  // positions to put
  int posInfo=sizeof(Record)*info->maxRecordFirstBlock;
  int posBlockInfo=sizeof(Record)*info->maxRecordFirstBlock+sizeof(info)*1;
  
  // copy HP_block_info and HP_info to the first block
  memcpy(data+posBlockInfo,&blockInfo,sizeof(blockInfo)); 
  memcpy(data+posInfo,info,sizeof(*info)); 

  // we changed the data of the block so set it dirty
  BF_Block_SetDirty(block);

  // set it so anyone can take it
  CALL_BF(BF_UnpinBlock(block),NULL); 

  // we delete the block
  BF_Block_Destroy(&block);

  return info;
}

// returns the HP_block_info from the block. On error returns NULL
HP_block_info HP_GetInfo(HP_info* hp_info,BF_Block**block,int flagFirst){
  HP_block_info blockInfo;
  int posBlockInfo;
  if(flagFirst==1)
    posBlockInfo=sizeof(Record)*hp_info->maxRecordFirstBlock+sizeof(hp_info)*1;
  else
    posBlockInfo=sizeof(Record)*hp_info->maxRecordPerBlock+sizeof(hp_info)*1;

  memcpy(&blockInfo,block+posBlockInfo,sizeof(block));

  return blockInfo;
}


int HP_CloseFile( HP_info* hp_info ){
  // close file
  CALL_BF(BF_CloseFile(hp_info->fileDesc),-1);
  
  free(hp_info);
  return 0;
}


int HP_InsertEntry(HP_info* hp_info, Record record){
  return -1;
  // int size;
  // CALL(BF_GetBlockCounter(hp_info->fileDesc,&size),-1);
  // BF_Block** block;
  // BF_Block_Init(block);
  // if(size==1){
  //   BF_GetBlock(hp_info->fileDesc,0,block);
  //   HP_block_info blockInfo= HP_GetInfo(hp_info,block,0);
  //   if(blockInfo.sizeOfRecords<hp_info->maxRecordFirstBlock){
  //     memcpy(block+blockInfo.sizeOfRecords,&record,sizeof(record));
  //     blockInfo.sizeOfRecords++;
  //     memcpy(block+blockInfo.)
  //     return 0;
  //   }
  // }
  // else{

  //   return 0;
  // }
  // BF_Block_Destroy(block);
}

int HP_GetAllEntries(HP_info* hp_info, int value){

  int numberOfVisitedBlocks=0;
  int file_desc = hp_info->fileDesc;
  
  // find number of blocks in this file
  int blocks_num;
  CALL_BF(BF_GetBlockCounter(file_desc, &blocks_num),-1);
  
  //find first block
  BF_Block* block;
  CALL_BF(BF_GetBlock(file_desc, 0, block), -1);

  //find hp_block_info of first block
  HP_block_info blockInfo = HP_GetInfo(hp_info, &block, 1);

  int block_records = blockInfo.numOfRecords;
  void* data;

  while(blocks_num>0) {
    numberOfVisitedBlocks++;
    
    BF_Block_Init(&block);
    data = BF_Block_GetData(block); 
    Record* rec = data;
    // checking the block's records
    for(int i=0; i<block_records; i++) {
      if(rec[i].id == value)
        printRecord(rec[i]);
    }
    //find next block to check
    block = blockInfo.nextBlock;
    //find next block's hp_block_info
    blockInfo = HP_GetInfo(hp_info, &block, 0);
    block_records = blockInfo.numOfRecords;

    blocks_num--;

  }

   return numberOfVisitedBlocks;
}

  // BF_Block* last_block = hp_info->lastBlock;
  // int max_record_first_block = hp_info->maxRecordFirstBlock;
  // int max_record_per_block = hp_info->maxRecordPerBlock;