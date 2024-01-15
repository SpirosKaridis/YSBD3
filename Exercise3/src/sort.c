#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"


bool shouldSwap(Record* rec1, Record* rec2) {
    // Implement your comparison logic here
    // For example, compare based on name and surname
    int cmp = strcmp(rec1->name, rec2->name);
    if (cmp == 0) {
        return strcmp(rec1->surname, rec2->surname) > 0;
    }
    return cmp > 0;
}


void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    CHUNK chunk;
    
    while (CHUNK_GetNext(&iterator, &chunk)) {
        printf("im here\n");
        sort_Chunk(&chunk);
    }
}

// Function to partition the array for Quicksort
int partition(Record* arr, int low, int high) {
    Record pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (shouldSwap(&arr[j], &pivot)) {
            i++;
            Record temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    Record temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;

    return i + 1;
}

// Quicksort algorithm
void quicksort(Record* arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

void sort_Chunk(CHUNK* chunk) {
    // Assuming chunk->recordsInChunk represents the total number of records in the chunk
    Record* records = (Record*)malloc(chunk->recordsInChunk * sizeof(Record));
    
    // Load all records from the chunk
    for (int i = 0; i < chunk->recordsInChunk; i++) {
        if (CHUNK_GetIthRecordInChunk(chunk, i, &records[i]) == -1) {
            // Handle error
            fprintf(stderr, "Error getting records from chunk.\n");
            free(records);
            return;
        }
    }
    
    // Apply Quicksort to the array of records
    quicksort(records, 0, chunk->recordsInChunk - 1);

    // Update the chunk with the sorted records
    for (int i = 0; i < chunk->recordsInChunk; i++) {
        if (CHUNK_UpdateIthRecord(chunk, i, records[i]) == -1) {
            // Handle error
            fprintf(stderr, "Error updating records in chunk.\n");
            free(records);
            return;
        }
    }

    free(records);
}