#include <merge.h>
#include <stdio.h>
#include <string.h>
#include "chunk.h"
#include "merge.h"
#include "record.h"
#include "bf.h"




CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator iterator;
    iterator.file_desc = fileDesc;
    iterator.current = 1; // Assuming the chunks start from block 1
    iterator.lastBlocksID = HP_GetIdOfLastBlock(fileDesc); // Initially unknown
    iterator.blocksInChunk = blocksInChunk;

    return iterator;
}

int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
    // Check if there are more chunks to iterate
    if (iterator->current <= iterator->lastBlocksID) {
        chunk->file_desc = iterator->file_desc;
        chunk->from_BlockId = iterator->current;

        if((iterator->current + iterator->blocksInChunk) >= iterator->lastBlocksID){//this is the last chunk
            chunk->to_BlockId = HP_GetIdOfLastBlock(iterator->file_desc);
            chunk->blocksInChunk = chunk->to_BlockId - chunk->from_BlockId + 1;

            chunk->recordsInChunk = 0;
            for(int i = chunk->from_BlockId; i <= chunk->to_BlockId; i++){
                chunk->recordsInChunk += HP_GetRecordCounter(iterator->file_desc, i);
            }
        }else{
            chunk->to_BlockId = iterator->current + iterator->blocksInChunk - 1;
            chunk->recordsInChunk = iterator->blocksInChunk * HP_GetMaxRecordsInBlock(iterator->file_desc);
            chunk->blocksInChunk = iterator->blocksInChunk;
        }
        
        // Increment the iterator for the next iteration
        iterator->current += iterator->blocksInChunk;

        return 1; // Success
    }

    return 0; // No more chunks to iterate
}

int CHUNK_GetIthRecordInChunk(CHUNK *chunk, int i, Record *record) {
    if (i >= 0 && i < chunk->recordsInChunk) {
        int blockId = chunk->from_BlockId + i / BF_BLOCK_SIZE;
        int cursor = i % BF_BLOCK_SIZE;

        BF_Block *bfBlock;
        BF_Block_Init(&bfBlock);
        CALL_BF(BF_GetBlock(chunk->file_desc, blockId, bfBlock));

        char *data = BF_Block_GetData(bfBlock);
        Record *blockRecords = (Record *)(data + sizeof(int)); // Skip the first int, which stores the number of records

        *record = blockRecords[cursor];

        CALL_BF(BF_UnpinBlock(bfBlock));
        BF_Block_Destroy(&bfBlock);

        return 0; // Success
    }

    return -1; // Invalid index
}

int CHUNK_UpdateIthRecord(CHUNK *chunk, int i, Record record) {
    if (i >= 0 && i < chunk->recordsInChunk) {
        int blockId = chunk->from_BlockId + i / BF_BLOCK_SIZE - 1;
        int cursor = i % BF_BLOCK_SIZE;

        BF_Block *bfBlock;
        BF_Block_Init(&bfBlock);
        CALL_BF(BF_GetBlock(chunk->file_desc, blockId, bfBlock));

        char *data = BF_Block_GetData(bfBlock);
        Record *blockRecords = (Record *)(data + sizeof(int)); // Skip the first int, which stores the number of records

        HP_UpdateRecord(chunk->file_desc, blockId, cursor + sizeof(int), record);
        //memcpy(data + cursor * sizeof(record) + sizeof(int), &record, sizeof(record));
        //blockRecords[cursor] = record;
        BF_Block_SetDirty(bfBlock);

        CALL_BF(BF_UnpinBlock(bfBlock));
        BF_Block_Destroy(&bfBlock);

        return 0; // Success
    }

    return -1; // Invalid index
}

void CHUNK_Print(CHUNK chunk) {
    printf("Chunk Information:\n");
    printf("File Descriptor: %d\n", chunk.file_desc);
    printf("From Block ID: %d\n", chunk.from_BlockId);
    printf("To Block ID: %d\n", chunk.to_BlockId);
    printf("Records in Chunk: %d\n", chunk.recordsInChunk);
    printf("Blocks in Chunk: %d\n", chunk.blocksInChunk);
}

CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {
    CHUNK_RecordIterator iterator;
    iterator.chunk = *chunk;
    iterator.currentBlockId = chunk->from_BlockId;
    iterator.cursor = 0;

    return iterator;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record *record) {
    if (iterator->currentBlockId <= iterator->chunk.to_BlockId) {
        BF_Block *bfBlock;
        BF_Block_Init(&bfBlock);
        CALL_BF(BF_GetBlock(iterator->chunk.file_desc, iterator->currentBlockId, bfBlock));

        char *data = BF_Block_GetData(bfBlock);
        Record *blockRecords = (Record *)(data + sizeof(int)); // Skip the first int, which stores the number of records

        *record = blockRecords[iterator->cursor];

        if (iterator->cursor < BF_BLOCK_SIZE - 1) {
            // Move to the next record in the block
            iterator->cursor++;
        } else {
            // Move to the next block
            iterator->currentBlockId++;
            iterator->cursor = 0;
        }

        CALL_BF(BF_UnpinBlock(bfBlock));
        BF_Block_Destroy(&bfBlock);

        return 0; // Success
    }

    return -1; // No more records in the chunk
}




