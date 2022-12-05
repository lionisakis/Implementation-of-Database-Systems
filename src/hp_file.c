#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){
  BF_ErrorCode err;
  // you create a file with file name
  if (err=BF_CreateFile(fileName)!=BF_OK){
    // if it is not created print the error
    BF_PrintError(err);
    return -1;
  }
  // we could not open the file 
  if(HP_OpenFile(fileName)==NULL)
    return -1;
  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  BF_ErrorCode err;
  int file_desc;
  // open the file that you created to put HP_info
  if (err=BF_OpenFile(fileName,&file_desc)!=BF_OK){
    // if it is not created print the error
    BF_PrintError(err);
    return -1;
  }

  return NULL ;
}


int HP_CloseFile( HP_info* hp_info ){
    return 0;
}

int HP_InsertEntry(HP_info* hp_info, Record record){
    return 0;
}

int HP_GetAllEntries(HP_info* hp_info, int value){
   return 0;
}

