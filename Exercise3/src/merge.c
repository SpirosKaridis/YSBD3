#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "merge.h"
#include "chunk.h"
#include "record.h"
#include "sort.h"

// Implement the merge sort algorithm
void mergeSort(Record *arr, int n) {
    if (n > 1) {
        int mid = n / 2;
        int leftSize = mid;
        int rightSize = n - mid;

        // Create left and right subarrays
        Record *left = (Record *)malloc(leftSize * sizeof(Record));
        Record *right = (Record *)malloc(rightSize * sizeof(Record));

        // Populate left and right subarrays
        memcpy(left, arr, leftSize * sizeof(Record));
        memcpy(right, arr + mid, rightSize * sizeof(Record));

        // Recursively sort the subarrays
        mergeSort(left, leftSize);
        mergeSort(right, rightSize);

        // Merge the sorted subarrays
        int i = 0, j = 0, k = 0;

        while (i < leftSize && j < rightSize) {
            if (shouldSwap(&left[i], &right[j])) {
                arr[k++] = right[j++];
            } else {
                arr[k++] = left[i++];
            }
        }

        while (i < leftSize) {
            arr[k++] = left[i++];
        }

        while (j < rightSize) {
            arr[k++] = right[j++];
        }

        // Free allocated memory
        free(left);
        free(right);
    }
}

// Assuming there is a function to write a record to a file
int writeRecordToFile(int fileDesc, Record *record) {
    // Seek to the end of the file to append the record
    if (lseek(fileDesc, 0, SEEK_END) == -1) {
        perror("Error seeking to the end of the file");
        return -1;
    }

    // Write the record to the file
    if (write(fileDesc, record, sizeof(Record)) == -1) {
        perror("Error writing record to file");
        return -1;
    }

    return 0;  // Return 0 for success, -1 for failure
}

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    // Calculate the total number of records in each merge step
    int totalRecordsInMerge = chunkSize * bWay;

    // Allocate memory for an array to hold the records
    Record *records = (Record *)malloc(totalRecordsInMerge * sizeof(Record));

    if (records == NULL) {
        // Handle memory allocation error
        fprintf(stderr, "Memory allocation error in merge function.\n");
        return;
    }

    // Create an array of record iterators, one for each chunk
    CHUNK_RecordIterator *recordIterators = (CHUNK_RecordIterator *)malloc(bWay * sizeof(CHUNK_RecordIterator));

    if (recordIterators == NULL) {
        // Handle memory allocation error
        fprintf(stderr, "Memory allocation error in merge function.\n");
        free(records);
        return;
    }

    // Initialize the record iterators
    for (int i = 0; i < bWay; i++) {
        recordIterators[i] = CHUNK_CreateRecordIterator(NULL);
    }

    // Iterate through the input chunks
    CHUNK_Iterator inputIterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);

    while (CHUNK_GetNext(&inputIterator, &recordIterators[0].chunk) == 0) {
        // Load records from each chunk into the records array
        for (int i = 0; i < bWay; i++) {
            if (CHUNK_GetNextRecord(&recordIterators[i], &records[i]) != 0) {
                // Handle error
                fprintf(stderr, "Error getting records from chunk.\n");
                free(records);
                free(recordIterators);
                return;
            }
        }

        // Sort the records array using merge sort
        mergeSort(records, totalRecordsInMerge);

        // Write the sorted records back to the output file
        for (int i = 0; i < totalRecordsInMerge; i++) {
            // Assuming there is a function to write a record to a file
            // You may need to replace this with the appropriate function
            // depending on your file structure
            if (writeRecordToFile(output_FileDesc, &records[i]) != 0) {
                // Handle error
                fprintf(stderr, "Error writing record to output file.\n");
                free(records);
                free(recordIterators);
                return;
            }
        }
    }

    // Free allocated memory
    free(records);
    free(recordIterators);
}

