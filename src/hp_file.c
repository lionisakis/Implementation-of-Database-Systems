#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call,returnCode)    \
{                                   \
  BF_ErrorCode code = call;         \
  if (code != BF_OK) {              \
    BF_PrintError(code);            \
    return returnCode;              \
  }                                 \
}

#define POS_HP_block_info BF_BLOCK_SIZE-sizeof(HP_block_info)-1
#define POS_HP_info BF_BLOCK_SIZE-sizeof(HP_block_info)-sizeof(HP_info)-1

int HP_CreateFile(char *fileName){
  // you create a file with file name
  CALL_BF(BF_CreateFile(fileName),-1);

  // create block info
  HP_block_info blockInfo;
  blockInfo.nextBlockNumber=-1;
  blockInfo.numOfRecords=0;

  int file_desc;
  // open the file that you created to put HP_info
  CALL_BF(BF_OpenFile(fileName,&file_desc),-1);

  // create the info to store
  HP_info info;

  info.fileDesc = -1;
  info.maxRecordFirstBlock = (BF_BLOCK_SIZE-sizeof(HP_info)-sizeof(HP_block_info)-1)/sizeof(Record);
  info.maxRecordPerBlock=(BF_BLOCK_SIZE-sizeof(HP_block_info)-1)/sizeof(Record);

  // initialize the block
  BF_Block* block;
  BF_Block_Init(&block);

  // make a block to store the data 
  CALL_BF(BF_AllocateBlock(file_desc,block),-1);
  void* data=BF_Block_GetData(block);

  // positions to put

  // copy HP_block_info and HP_info to the first block
  memcpy(data+POS_HP_block_info,&blockInfo,sizeof(HP_block_info));
  memcpy(data+POS_HP_info,&info,sizeof(HP_info)); 

  // we changed the data of the block so set it dirty
  BF_Block_SetDirty(block);

  // set it so anyone can take it
  CALL_BF(BF_UnpinBlock(block),-1); 

  // we delete the block
  BF_Block_Destroy(&block);

  // close file
  CALL_BF(BF_CloseFile(file_desc),-1);

  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  int file_desc;
  // open the file that you created to put HP_info
  CALL_BF(BF_OpenFile(fileName,&file_desc),NULL);


  // find the first block and take the data
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(file_desc,0,block),NULL);
  void* data=BF_Block_GetData(block);

  // take the info data from the block
  HP_info* info=malloc(sizeof(*info));
  if(info==NULL){
    printf("Not enough size for malloc\n");
    return NULL;
  }

  memcpy(info,data+POS_HP_info,sizeof(HP_info));
  info->fileDesc=file_desc;
  memcpy(data+POS_HP_info,info,sizeof(HP_info));

  // unpin the block and destroy it
  CALL_BF(BF_UnpinBlock(block),NULL);
  BF_Block_Destroy(&block);
  return info;
}

// returns the HP_block_info from the block. On error returns NULL
void HP_Get_HP_Block_Info(void* data,HP_block_info* blockInfo){
  // the HP_block_info is always at the last bytes
  memcpy(blockInfo,data+POS_HP_block_info,sizeof(HP_block_info));
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
  // find the first block and get the HP_block_info
  BF_GetBlock(hp_info->fileDesc,size-1,block);
  void* data=BF_Block_GetData(block);
  HP_block_info blockInfo;
  HP_Get_HP_Block_Info(data,&blockInfo);
  int insertMethod;
  // if the size==1 then you insert in the first block
  if(size==1){
    // if there is room in this block, add it here
    if(blockInfo.numOfRecords < hp_info->maxRecordFirstBlock){
      insertMethod=0;
    }
    else{
      insertMethod=1;
    }
  }
  else{
    if(blockInfo.numOfRecords < hp_info->maxRecordPerBlock){
      insertMethod=0;
    }
    else{
      insertMethod=1;
    }
  }

  if (insertMethod==0){
   // copy the Record to the block 
    memcpy(data+blockInfo.numOfRecords*sizeof(Record),&record,sizeof(Record));
    // and change the HP_block_info for the numOfRecords
    blockInfo.numOfRecords++;
    memcpy(data+POS_HP_block_info,&blockInfo,sizeof(HP_block_info));
    // the data changed
    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
    return size-1;
  }
  else{
    // set that there will be a next block
    blockInfo.nextBlockNumber=size;
    memcpy(data+POS_HP_block_info,&blockInfo,sizeof(HP_block_info));
    // the data changed
    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);

    BF_Block* newBLock;
    BF_Block_Init(&newBLock);
    CALL_BF(BF_AllocateBlock(hp_info->fileDesc,newBLock),-1);
    void* newData=BF_Block_GetData(newBLock);
  
    // copy the Record to the new block 
    memcpy(newData,&record,sizeof(Record));

    HP_block_info newBlockInfo;
    newBlockInfo.nextBlockNumber=-1;
    newBlockInfo.numOfRecords=1;

    // and change the HP_block_info for the numOfRecords 
    memcpy(newData+POS_HP_block_info,&newBlockInfo,sizeof(HP_block_info));

    // the data changed
    BF_Block_SetDirty(newBLock);
    BF_UnpinBlock(newBLock);
    BF_Block_Destroy(&newBLock);
    
    return size;
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
  HP_block_info blockInfo;
  HP_Get_HP_Block_Info(data,&blockInfo);

  int block_records = blockInfo.numOfRecords;

  while(blocks_num>0) {
    numberOfVisitedBlocks++;
    Record* rec = (Record*)data;
    // checking the block's records
    for(int i=0; i<block_records; i++) {
      if(rec[i].id == value)
        printRecord(rec[i]);
    }
    //find next block to check
    if (blockInfo.nextBlockNumber!=-1){
      // find the next block
      CALL_BF(BF_UnpinBlock(block),-1);
      int next = blockInfo.nextBlockNumber;
      CALL_BF(BF_GetBlock(hp_info->fileDesc,next,block),-1);
      data = BF_Block_GetData(block); 
      
      //find next block's hp_block_info
      HP_Get_HP_Block_Info(data,&blockInfo);

      block_records = blockInfo.numOfRecords;
    }
      
    blocks_num--;

  }
  // set it so anyone can take it
  CALL_BF(BF_UnpinBlock(block),-1);

  BF_Block_Destroy(&block);
  return numberOfVisitedBlocks;
}