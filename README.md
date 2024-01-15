Μέλη Ομάδας: 
    Κοντοχρήστος Χρήστος Α.Μ.: 1115202000090
        Υλοποίησα το CHUNK library.
    Νικέλλη Εμμανουέλα Α.Μ.: 1115202000152
        Υλοποίησα τo SORT library.
    Καρύδης Σπυρίδων Α.Μ.: 1115202000256
        Υλοποίησα τo MERGE.


Συνομιλίες με το ChatGpt:
    https://chat.openai.com/share/7b3fb7d9-9d0c-4479-91a6-24a59b8d3fbb

    Για το Sort:
    https://chat.openai.com/share/98b1ba9a-bde4-4f99-b229-e7c42ea10e53

    https://chat.openai.com/share/c05bc7d8-c54c-4a86-b984-0a784feefef9

    https://chat.openai.com/share/303e4113-6264-49f9-8f58-d2040f17b3ec


Γενικά σχόλια:


Για το sort library:
    Οι αλλαγές που έκανα στις συναρτήσεις της Sort που παρήγαγε το ChatGPT ήταν πολύ μικρές. Συγκεκριμένα, άλλαξα τον τελεστή σύγκρισης στην shouldSwap ώστε η ταξινόμιση να γίνεται σε αύξουσα σειρά αντί για φθίνουσα που ήταν αρχικά και έκανα αλλαγή της συνθήκης "if (CHUNK_GetIthRecordInChunk(chunk, i, &records[i]) == 0)" στην sort_Chunk σε "if (CHUNK_GetIthRecordInChunk(chunk, i, &records[i]) == -1)" ώστε να ταιριάζει με την υλοποίηση της CHUNK_GetIthRecordInChunk. Ο υπόλοιπος κώδικας είναι ακριβώς αυτός που εφτιαξε το ChatGPT. Επιπλέον, χρησιμοποίησα την quicksort ως μέθοδο ταξινόμισης.



Εγώ υλοποίησα τις συναρτήσεις του chunk.c (και βοήθησα στην υλοποίηση της merge.c) .


chunk.c :

Όλες οι συναρτήσεις υλοποιήθηκαν κυρίως από το ChatGPT με κάποιες μικρές αλλαγές.
Οι αλλαγές αυτές είναι οι εξής :



Στην CHUNK_Iterator CHUNK_CreateIterator() το ChatGPT αρχικά είχε αρκικοποιήσει την μεταβλητή iterator.lastBlocksID = 0; και εγώ το άλλαξα ώστε
να παίρνει το id του τελευταίου μπλοκ (χρησιμοποιώντας την HP_GetIdOfLastBlock) σε iterator.lastBlocksID = HP_GetIdOfLastBlock(fileDesc); .



Στην int CHUNK_GetNext() το ChatGPT δεν λάμβανε υπόψη την περίπτωση του τελευταίου chunk οπότε το πρόσθεσα ένα if case για αυτή την περίπτωση.
Δηλαδή αν το chunk είναι το τελευταίο τότε αρχικοποιεί το struct σωστά διότι το τελευταίο chunk μπορεί να μην έχει το max αριθμό από blocks/records 
που θα έχουν τα υπόλοιπα.



Στις CHUNK_GetIthRecordInChunk() και CHUNK_UpdateIthRecord() άλλαξα τα εξής : 

Αυτές τις γραμμές σε :

int blockId = chunk->from_BlockId + i / HP_GetMaxRecordsInBlock(chunk->file_desc);
int cursor = i % HP_GetMaxRecordsInBlock(chunk->file_desc);

ενώ πρίν ήταν :

int blockId = chunk->from_BlockId + i / BF_BLOCK_SIZE;
int cursor = i % BF_BLOCK_SIZE;

και τη γραμμή Record *blockRecords = (Record *)(data); ενώ πρίν ήταν Record *blockRecords = (Record *)(data + sizeof(int));

Οι αλλαγές σε αυτές τις 2 συναρτήσεις ήταν οι ίδιες.



Στην CHUNK_RecordIterator CHUNK_CreateRecordIterator() απλά πρόσθεσα την περίπτωση που το όρισμα της συνάρτησης είναι ίσο με NULL
ώστε να γίνεται handled σωστά αυτή η περίπτωση.



Στην int CHUNK_GetNextRecord() άλλαξα τη συνθήκη του 1ου εμφωλευμένου if case από if (iterator->cursor < BF_BLOCK_SIZE - 1) σε
if (iterator->cursor < HP_GetMaxRecordsInBlock(iterator->chunk.file_desc)).





merge.c

Παρατήρηση: ενώ η λογική της merge() είναι σωστή δεν δουλεύει σωστά.


Στην merge συνάρτηση έγιναν οι εξής αλλαγές :

Στη γραμμή 152 άλλαξε σε if (HP_InsertEntry(output_FileDesc, records[i]) != 1)  από if (writeRecordToFile(output_FileDesc, &records[i]) != 0) 

Το int totalRecordsInMerge = chunkSize * bWay; άλλαξε σε int totalRecordsInMerge = chunkSize * HP_GetMaxRecordsInBlock(input_FileDesc) * bWay;

Το ChatGPT έκανε initialize το recordIterators[] array δίνοντας NULL όρισμα στην CHUNK_CreateRecordIterator και άλλαξε ώστε να μπάινει το 
κάθε chunk σωστά στο array (γραμμές 103 - 132). 