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
  int file_desc;
  // open the file that you created to put HP_info
  CALL_BF(BF_OpenFile(fileName,&file_desc),NULL);

  int blocks;
  CALL_BF(BF_GetBlockCounter(file_desc,&blocks),NULL);
  // check if this file has already an HP_info
  if(blocks>0){
    // find the first block and take the data
    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(file_desc,0,block),NULL);
    void* data=BF_Block_GetData(block);

    // take the info data from the block
    HP_info* info=malloc(sizeof(*info));

    int maxRecordFirstBlock = (BF_BLOCK_SIZE-sizeof(*info)-sizeof(HP_block_info))/sizeof(Record);
    int posInfo=maxRecordFirstBlock*sizeof(Record);
    memcpy(info,data+posInfo,sizeof(*info));

    // unpin the block and destroy it
    CALL_BF(BF_UnpinBlock(block),NULL);
    BF_Block_Destroy(&block);
    printf("info:%d %d %d\n",info->fileDesc,info->maxRecordFirstBlock,info->maxRecordPerBlock);
    printf("pos info: %d\n",posInfo);
    return info;
  }

  // create block info
  HP_block_info blockInfo;
  blockInfo.nextBlock=NULL;
  blockInfo.numOfRecords=0;

  // create the info to store
  HP_info* info= malloc(sizeof(*info));
  if(info==NULL)
    return NULL;

  info->fileDesc = file_desc;
  info->maxRecordFirstBlock = (BF_BLOCK_SIZE-sizeof(*info)-sizeof(blockInfo))/sizeof(Record);
  info->maxRecordPerBlock=(BF_BLOCK_SIZE-sizeof(blockInfo))/sizeof(Record);

  // initialize the block
  BF_Block* block;
  BF_Block_Init(&block);

  // make a block to store the data 
  CALL_BF(BF_AllocateBlock(file_desc,block),NULL);
  void* data=BF_Block_GetData(block);

  // positions to put
  int posInfo=sizeof(Record)*info->maxRecordFirstBlock;
  int posBlockInfo=sizeof(Record)*info->maxRecordFirstBlock+sizeof(*info)*1;
  printf("posBlockInfo %d\n",posBlockInfo);
  // copy HP_block_info and HP_info to the first block
  memcpy(data+posBlockInfo,&blockInfo,sizeof(blockInfo));
  memcpy(data+posInfo,info,sizeof(*info)); 
  printf("pos info: %d sizeof(info):%ld\n",posInfo,sizeof(*info));

  // we changed the data of the block so set it dirty
  BF_Block_SetDirty(block);

  // set it so anyone can take it
  CALL_BF(BF_UnpinBlock(block),NULL); 

  // we delete the block
  BF_Block_Destroy(&block);

  return info;
}

// returns the HP_block_info from the block. On error returns NULL
HP_block_info HP_Get_HP_Block_Info(HP_info* hp_info,void* data,int flagFirst){
  HP_block_info blockInfo;
  int posBlockInfo;
  if(flagFirst==1)
    posBlockInfo=sizeof(Record)*hp_info->maxRecordFirstBlock+sizeof(*hp_info);
  else
    posBlockInfo=sizeof(Record)*hp_info->maxRecordPerBlock;
  
  memcpy(&blockInfo,data+posBlockInfo,sizeof(blockInfo));
  return blockInfo;
}


int HP_CloseFile( HP_info* hp_info ){
  // close file
  CALL_BF(BF_CloseFile(hp_info->fileDesc),-1);
  
  free(hp_info);

  return 0;
}


int HP_InsertEntry(HP_info* hp_info, Record record){
  int size;
  // get the size to find the last block 
  CALL_BF(BF_GetBlockCounter(hp_info->fileDesc,&size),-1);
  BF_Block* block;
  BF_Block_Init(&block);
  // if the size==1 then you insert in the first block
  if(size==1){
    // find the first block and get the HP_block_info
    BF_GetBlock(hp_info->fileDesc,0,block);
    void* data=BF_Block_GetData(block);
    HP_block_info blockInfo= HP_Get_HP_Block_Info(hp_info,data,1);

    // if there is room in this block, add it here
    if(blockInfo.numOfRecords<hp_info->maxRecordFirstBlock){
      // copy the Record to the block 

      memcpy(data+blockInfo.numOfRecords*sizeof(Record),&record,sizeof(Record));
      
      // and change the HP_block_info for the numOfRecords
      blockInfo.numOfRecords++;
      memcpy(data+hp_info->maxRecordFirstBlock*sizeof(Record)+sizeof(HP_info),&blockInfo,sizeof(HP_block_info));

      // the data changed
      BF_Block_SetDirty(block);
      BF_UnpinBlock(block);
      BF_Block_Destroy(&block);
      return 0;
    }
    // else{

    // }
  }
  else{
    // if(){

    // }
    // else{

    // }
    return 0;
  }
  return -1;
}

int HP_GetAllEntries(HP_info* hp_info, int value){

  int numberOfVisitedBlocks=0;
  int file_desc = hp_info->fileDesc;
 
  // find number of blocks in this file
  int blocks_num;
  CALL_BF(BF_GetBlockCounter(file_desc, &blocks_num),-1);
  
  //find first block
  BF_Block* block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(file_desc, 0, block), -1);

  //find hp_block_info of first block
  void* data = BF_Block_GetData(block); 
  HP_block_info blockInfo = HP_Get_HP_Block_Info(hp_info, data, 1);

  int block_records = blockInfo.numOfRecords;

  while(blocks_num>0) {
    numberOfVisitedBlocks++;
    printf("block_records %d\n",block_records);
    Record* rec = (Record*)data;
    // checking the block's records
    for(int i=0; i<block_records; i++) {
      // if(rec[i].id == value)
      printRecord(rec[i]);
    }

    //find next block to check
    if (blockInfo.nextBlock!=NULL){
      block = blockInfo.nextBlock;
      //find next block's hp_block_info
      blockInfo = HP_Get_HP_Block_Info(hp_info, data, 0);

      block_records = blockInfo.numOfRecords;
      data = BF_Block_GetData(block); 
    }
      
    blocks_num--;

  }
  // set it so anyone can take it
  CALL_BF(BF_UnpinBlock(block),-1);

  BF_Block_Destroy(&block);
  return numberOfVisitedBlocks;
}

  // BF_Block* last_block = hp_info->lastBlock;
  // int max_record_first_block = hp_info->maxRecordFirstBlock;
  // int max_record_per_block = hp_info->maxRecordPerBlock;