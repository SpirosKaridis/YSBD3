#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

//just prints metadata//

void printInfo(HP_info *info){
  printf("\nPrinting info\n");
  if(info == NULL){
    printf("what u want me to do :/ \n");
    return;
  }
  printf("blocksize: %d\n", info->block_size);
  printf("blocid: %d\n", info->block_id);
  printf("records: %d\n", info->records);
  printf("size of file: %d\n", info->size_of_file);
}

/////////////////////////////////////////////////////////////////////////

int HP_CreateFile(char *fileName){

    int fileDesc;
    void* data;

    BF_Block *block;
    BF_Block_Init(&block); //init the block

    CALL_BF(BF_CreateFile(fileName));  //create the file of blocks
    CALL_BF(BF_OpenFile(fileName,&fileDesc)); //open the file of blocks
    CALL_BF(BF_AllocateBlock(fileDesc, block)); //allocate memory for the first block
    
    //the newly created block will contain the metadata of the heap file 
    data = BF_Block_GetData(block); 
    HP_info info = {BF_BLOCK_SIZE/sizeof(Record),0,1,0}; //initialize the metadata
    memcpy(data,&info,sizeof(info));

    BF_Block_SetDirty(block); //set the block to dirty 
    CALL_BF(BF_UnpinBlock(block)); //unpin the block 
    BF_Block_Destroy(&block); //destroy the block
    CALL_BF(BF_CloseFile(fileDesc)); //close the file 

    return 0;
}

/////////////////////////////////////////////////////////////////////////

HP_info* HP_OpenFile(char *fileName, int *file_desc){
  HP_info* hpInfo;
  void *data;   
  BF_Block *block;      //initiallizing local variable for block
  BF_Block_Init(&block);

  BF_ErrorCode err;
  if((err = BF_OpenFile(fileName, file_desc)) != BF_OK){//open file with bf function
    BF_PrintError(err);
    return NULL;
  }
  if((err = BF_GetBlock(*file_desc, 0, block)) != BF_OK){//get the block containing the metatada of the file
    BF_PrintError(err);
    return NULL;
  }
  
  data = BF_Block_GetData(block);           //get the address of the info
  hpInfo = data;
  
  if((err = BF_UnpinBlock(block)) != BF_OK){//unpin block
    BF_PrintError(err);
    return NULL;
  }
  
  BF_Block_Destroy(&block);                 //destroy 

  return hpInfo;                            //return the info 
}

/////////////////////////////////////////////////////////////////////////

int HP_CloseFile(int file_desc, HP_info *hp_info){
  BF_Block *block;
  BF_Block_Init(&block);    //initiallizing local variable for block

  int count;
  CALL_BF(BF_GetBlockCounter(file_desc, &count));

  //unpin all blocks, in case some are left pinned
  for(int i = 0; i < count; i++){     //itterate all file blocks           
    CALL_BF(BF_GetBlock(file_desc, i, block));
    CALL_BF(BF_UnpinBlock(block));
  }
  
  BF_Block_Destroy(&block);           //destroy local variable
  CALL_BF(BF_CloseFile(file_desc));   //close file using bf close file
  return 0;
}

/////////////////////////////////////////////////////////////////////////

//just prints record
void PrintData(Record *rec){

  printf("id : %d\n",rec->id);
  printf("name : %s\n",rec->name);
  printf("surname : %s\n",rec->surname);
  printf("city : %s\n",rec->city);

  return;
}

/////////////////////////////////////////////////////////////////////////

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){

  BF_Block *first_block;
  BF_Block *block;
  void *data;
  int counter;
  HP_block_info *bi_ptr;
  BF_Block_Init(&block);
  BF_Block_Init(&first_block);

  CALL_BF(BF_GetBlockCounter(file_desc, &counter)); //get the number of blocks in the file 

  CALL_BF(BF_GetBlock(file_desc,0,first_block)); //upload the first block of the file in the memory

  //the case where the only block in the file is the one carying the metadata
  if(counter == 1){ 
    //allocate memory for the new block 
    CALL_BF(BF_AllocateBlock(file_desc, block));
    //get the pointer to the data
    data = BF_Block_GetData(block);
    //initialize and add the metadata of the block to the end of the block
    HP_block_info block_info = {1,sizeof(record) + sizeof(HP_block_info),counter}; //initialize the HP_block_info struct members
    int offset = BF_BLOCK_SIZE - sizeof(block_info); // offset = size of the block in bytes - size of the block info in bytes 
    memcpy(data + offset,&block_info,sizeof(HP_block_info));
    //add the new record
    memcpy(data,&record,sizeof(record));
    //update the metadata of the file
    hp_info->records++;
    hp_info->size_of_file++;

    BF_Block_SetDirty(first_block); //set the first block to dirty
    CALL_BF(BF_UnpinBlock(first_block)); //unpin the first block
    BF_Block_Destroy(&first_block); //destroy the first block
    BF_Block_SetDirty(block); //set the block to dirty 
    CALL_BF(BF_UnpinBlock(block)); //unpin the block 
    BF_Block_Destroy(&block); //destroy the block
    return 0;
  }

  CALL_BF(BF_GetBlock(file_desc,counter-1,block));
  data = BF_Block_GetData(block);
  bi_ptr = data + BF_BLOCK_SIZE - sizeof(HP_block_info); //get the block info metadata

  //the case where the record fits in the last block of the file
  if(BF_BLOCK_SIZE-(bi_ptr->bytes_occupied) >= sizeof(record)){ 
    //offset = all the occupied bytes in the block - the size of the block info in bytes 
    int offset = (bi_ptr->bytes_occupied) - sizeof(HP_block_info);
    //add the new record to the according spot at the block 
    memcpy(data + offset,&record,sizeof(record));
    bi_ptr->entries++;
    bi_ptr->bytes_occupied += sizeof(record); 
    hp_info->records++;

  }
  //the case where record doesn't fit to the block -> add a new block
  else{
    //unpin the previous block
    CALL_BF(BF_UnpinBlock(block)); 
    //allocate memory for the new block 
    CALL_BF(BF_AllocateBlock(file_desc, block));
    //get the pointer to the data
    data = BF_Block_GetData(block);
    //initialize and add the metadata of the block to the end of the block
    HP_block_info block_info = {1,sizeof(record) + sizeof(HP_block_info),counter}; //initialize the HP_block_info struct members
    int offset = BF_BLOCK_SIZE - sizeof(block_info); // offset = size of the block in bytes - size of the block info in bytes
    memcpy(data + offset,&block_info,sizeof(block_info));
    //add the new record
    memcpy(data,&record,sizeof(record));
    //update the metadata of the file
    hp_info->records++;
    hp_info->size_of_file++;

  } 

  BF_Block_SetDirty(first_block); //set the first block to dirty
  CALL_BF(BF_UnpinBlock(first_block)); //unpin the first block
  BF_Block_Destroy(&first_block); //destroy the first block
  BF_Block_SetDirty(block); //set the block to dirty 
  CALL_BF(BF_UnpinBlock(block)); //unpin the block 
  BF_Block_Destroy(&block); //destroy the block 

  return 0;
}

/////////////////////////////////////////////////////////////////////////

int HP_GetAllEntries(int file_desc, HP_info *hp_info, int value) {
  int blocks_num;
  void *data;
  Record *rec;
  int count = 0;

  BF_GetBlockCounter(file_desc, &blocks_num);
  BF_Block *block; // initialize of local variable
  BF_Block_Init(&block);

  for (int i = 1; i < blocks_num; i++) {  //we iterate through the number of blocks
    CALL_BF(BF_GetBlock(file_desc, i, block));
    data = BF_Block_GetData(block); //we load the data of every block on the data pointer
    rec = data;
    int offset = BF_BLOCK_SIZE - sizeof(HP_block_info); // since the sizes of records vary we offset in order to get each record
    HP_block_info *info = data + offset;  //calculating each record size
    for (int j = 0; j < info->entries; j++)
    {
      if (rec[j].id == value) // comparing the record id to the given value
      {
        count++;  // count to determine the number of valid records
        printRecord(rec[j]);
      }
    }
    CALL_BF(BF_UnpinBlock(block));
  }

  BF_Block_Destroy(&block);

  return count;
}
