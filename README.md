Team members:
-
-
    Spiros Karidis     implemeted MERGE.


ChatGpt Conversation:
    https://chat.openai.com/share/7b3fb7d9-9d0c-4479-91a6-24a59b8d3fbb

    
Για το Sort:
https://chat.openai.com/share/98b1ba9a-bde4-4f99-b229-e7c42ea10e53

https://chat.openai.com/share/c05bc7d8-c54c-4a86-b984-0a784feefef9

https://chat.openai.com/share/303e4113-6264-49f9-8f58-d2040f17b3ec


General comments:


For the sort library:
    The changed done to the functions of Sort produced by ChatGPT were minimal. Specificaly, changed the comparison operator of shouldSwap so that the sorting is done in ascending order instead of the initial descending order and made a change of the condition "if (CHUNK_GetIthRecordInChunk(chunk, i, &records[i]) == 0)" in sort_Chunk to "if (CHUNK_GetIthRecordInChunk(chunk, i, &records[i]) == -1)" so that it matches with the implementation of CHUNK_GetIthRecordInChunk. The rest of the code is the same as the one made by ChatGPT. Additionally quicksort was used as a sorting method.



merge.c

Notied: while the logic of merge() is correct it doesn't function properly.


On the merge function the following changes were done:

On line 152 cahnged to  if (HP_InsertEntry(output_FileDesc, records[i]) != 1)  from if (writeRecordToFile(output_FileDesc, &records[i]) != 0) 

The int totalRecordsInMerge = chunkSize * bWay; changed to int totalRecordsInMerge = chunkSize * HP_GetMaxRecordsInBlock(input_FileDesc) * bWay;

ChatGPT did initialize the recordIterators[] array giving NULL argument to CHUNK_CreateRecordIterator and changed so that every chunk is inserted correctly in the array (lines 103-132).
