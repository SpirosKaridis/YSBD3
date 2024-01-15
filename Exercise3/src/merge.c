#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include "record.h"
#include "bf.h"
#include "chunk.h"



void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc ) {

    CHUNK_Iterator chunkIterator = CHUNK_CreateIterator(input_FileDesc, chunkSize); // Create a CHUNK_Iterator for input file
    CHUNK_RecordIterator recordIterators[bWay]; // Create an array of CHUNK_RecordIterator for each chunk

    
    for (int i = 0; i < bWay; ++i) { // Initialize the record iterators
        if (recordIterators[i].currentBlockId <= chunkIterator.lastBlocksID) {// Check if there are more blocks in the current chunk
            recordIterators[i] = CHUNK_CreateRecordIterator(&chunkIterator.current);
        }
        else {
            recordIterators[i] = CHUNK_CreateRecordIterator(NULL); // If the chunk is exhausted, set a dummy record
        }
        }

        while (/* Condition based on your merging logic */) {// Continue merging until all chunks are processed

            int minIndex = findMinIndex(recordIterators, bWay); // Find the index of the chunk with the smallest current record

            mergeIntoOutput(recordIterators[minIndex].currentBlockId, output_FileDesc); // Merge the smallest record into the output block

            if (recordIterators[minIndex].currentBlockId <= chunkIterator.lastBlocksID) {// Move the corresponding record iterator to the next record
                CHUNK_MoveToNextRecord(&recordIterators[minIndex]);
            }

          
            if (recordIterators[minIndex].currentBlockId > chunkIterator.lastBlocksID) {   // If the current chunk is exhausted, load the next chunk
                CHUNK_AdvanceIterator(&chunkIterator); // Load the next chunk
                recordIterators[minIndex] = CHUNK_CreateRecordIterator((chunkIterator.current != NULL) ? &chunkIterator.current : NULL); // Reset the record iterator for the exhausted chunk
            }

          
            if (/* Condition to check if the output block is full */) { // Check if the output block is full, and write it to the output file if necessary
                writeOutputBlockToDisk(output_FileDesc, outputBlock);
            }
        }

        // Handle any remaining data in the output block
        if (/* Condition to check if there is remaining data in the output block */) {
            writeOutputBlockToDisk(output_FileDesc, outputBlock);
        }

        // Cleanup and close file descriptors if necessary
    }

   
    int findMinIndex(CHUNK_RecordIterator iterators[], int size) {  // Utility function to find the index of the chunk with the smallest current record
        int minIndex = -1;
        Record minRecord; // Initialize with a dummy record or handle NULL records

        for (int i = 0; i < size; ++i) {
            if ((iterators[i].currentBlockId != NULL) && (minIndex == -1 || compareRecords(iterators[i].currentBlockId, minRecord) < 0)) {
                minIndex = i;
                minRecord = iterators[i].currentBlockId;
            }
        }

        return minIndex;
    }

    // Utility function to compare two records; adjust based on your RECORD structure
    int compareRecords(Record record1, Record record2) {
        // Implementation depends on the specifics of your RECORD structure
        // Return a negative value if record1 is smaller, positive if larger, 0 if equal
    }

    // Utility function to merge a record into the output block
    void mergeIntoOutput(Record record, int output_FileDesc)
    {
        // Implementation depends on the specifics of your RECORD structure
        // Add the record to the output block
    }
