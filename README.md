# IMPLEMENTATION OF DATABASE SYSTEM

authors: 
- Despotidou Kalliopi Christina(Κallistina) (sdi2000045)
- Lionis Emmanouil Georgios(Akis) (sdi1900103)

# Summary
This project takes some Records and then saves them in data.db file stores with a heap or a hash table. Also, you can make a secondary hash table that saves the data in index.db. This way, we allocate blocks and data, and then someone can take the data from data.db or index.db and print all the entries. Also we have made some Statistics for some variations.

# Report
When we run the ht_main (the primary hashtable implementation), we observe that while records with integer ids from 1 to n (n= RECORDS_NUM) are inserted linearly, they are evenly distributed in the hastable blocks. In this way, some buckets can't avoid overflow, so new blocks are created.

### Run ht_main 
```
./build/ht_main  
(165,Sofia,Michas,Los Angeles)
FILE STATISTICS

This file has 40 blocks
Bucket 0 Max Records: 5
Bucket 0 Average Records: 5.000000
Bucket 0 Min Records: 5
 ~~~ 
Bucket 1 Max Records: 5
Bucket 1 Average Records: 5.000000
Bucket 1 Min Records: 5
 ~~~ 
Bucket 2 Max Records: 5
Bucket 2 Average Records: 5.000000
Bucket 2 Min Records: 5
 ~~~ 
Bucket 3 Max Records: 5
Bucket 3 Average Records: 5.000000
Bucket 3 Min Records: 5
 ~~~ 
Bucket 4 Max Records: 5
Bucket 4 Average Records: 5.000000
Bucket 4 Min Records: 5
 ~~~ 
Bucket 5 Max Records: 5
Bucket 5 Average Records: 5.000000
Bucket 5 Min Records: 5
 ~~~ 
Bucket 6 Max Records: 5
Bucket 6 Average Records: 5.000000
Bucket 6 Min Records: 5
 ~~~ 
Bucket 7 Max Records: 5
Bucket 7 Average Records: 5.000000
Bucket 7 Min Records: 5
 ~~~ 
Bucket 8 Max Records: 5
Bucket 8 Average Records: 5.000000
Bucket 8 Min Records: 5
 ~~~ 
Bucket 9 Max Records: 5
Bucket 9 Average Records: 5.000000
Bucket 9 Min Records: 5
 ~~~ 
Average blocks per bucket: 4.000000
Buckets with overflow blocks: 10
Bucket 0 Overflow Blocks: 3
Bucket 1 Overflow Blocks: 3
Bucket 2 Overflow Blocks: 3
Bucket 3 Overflow Blocks: 3
Bucket 4 Overflow Blocks: 3
Bucket 5 Overflow Blocks: 3
Bucket 6 Overflow Blocks: 3
Bucket 7 Overflow Blocks: 3
Bucket 8 Overflow Blocks: 3
Bucket 9 Overflow Blocks: 3
```

When we run the sht_main (the secondary hashtable implementation), we observe that while info with records of integer ids from 1 to n (n= MAX_RECORD) are NOT inserted linearly because the hashValue function divides the info data depending of the records' name, they are NOT evenly distributed in the secondary hastable blocks and some buckets remain empty. As a result, we can find easier the block we search for and we use less memory than we need in the sht. In the other hand, many info data are inserted in the same bucket and as result many new blocks are created in the very same bucket while other buckets are empty. In this way the worst case scenario complexity O(n) may occur in compare with the primary hashtable. Also, two or more records may have the same name domain, so we can seperate them and as a result in the getAllEntries function we may print two or more times the same record: when we search in the secondary hastable the block's info with the given name we find (name, blockId) info couple and nothing else about this record. Then we visit the block with id=blockId in the primary hashtable and look for the record in the  block's record table for the record with the given name and we print as many records have this name (maybe 2 or more). Then we go back in the secondary hashtable and look if any other info data from the info's table has the given name. If yes, we repeat the same actions in the primary hashtable. As a result, we don't know which records were already printed because we don't have their id, and we eventually print them again.

### Run sht_main 
```
./build/sht_main  
Insert Entries
RUN PrintAllEntries for name Vagelis
(20,Vagelis,Rezkalla,Los Angeles)
(40,Vagelis,Michas,Amsterdam)
(38,Vagelis,Mailis,Tokyo)
(20,Vagelis,Rezkalla,Los Angeles)
(40,Vagelis,Michas,Amsterdam)
(46,Vagelis,Rezkalla,Athens)
(59,Vagelis,Halatsis,London)
(89,Vagelis,Berreta,Hong Kong)
(61,Vagelis,Mailis,San Francisco)
(71,Vagelis,Koronis,Athens)
(61,Vagelis,Mailis,San Francisco)
(71,Vagelis,Koronis,Athens)
(78,Vagelis,Berreta,Amsterdam)
(59,Vagelis,Halatsis,London)
(89,Vagelis,Berreta,Hong Kong)
(94,Vagelis,Michas,San Francisco)
(109,Vagelis,Koronis,Munich)
(111,Vagelis,Svingos,Amsterdam)
(113,Vagelis,Gaitanis,Hong Kong)
(122,Vagelis,Michas,Athens)
(132,Vagelis,Koronis,San Francisco)
(122,Vagelis,Michas,Athens)
(132,Vagelis,Koronis,San Francisco)
(146,Vagelis,Rezkalla,Hong Kong)
(152,Vagelis,Mailis,London)
(154,Vagelis,Gaitanis,Amsterdam)
(155,Vagelis,Michas,Miami)
(177,Vagelis,Halatsis,London)
(179,Vagelis,Svingos,Hong Kong)
(186,Vagelis,Koronis,Amsterdam)
FILE STATISTICS-SHT

This file has 11 blocks
Bucket 0 Max INFO: 12
Bucket 0 Average INFO: 12.000000
Bucket 0 Min INFO: 12
 ~~~ 
Bucket 1 Max INFO: 24
Bucket 1 Average INFO: 21.666667
Bucket 1 Min INFO: 17
 ~~~ 
Bucket 2 Max INFO: 0
Bucket 2 Average INFO: 0.000000
Bucket 2 Min INFO: 0
 ~~~ 
Bucket 3 Max INFO: 12
Bucket 3 Average INFO: 12.000000
Bucket 3 Min INFO: 12
 ~~~ 
Bucket 4 Max INFO: 16
Bucket 4 Average INFO: 16.000000
Bucket 4 Min INFO: 16
 ~~~ 
Bucket 5 Max INFO: 21
Bucket 5 Average INFO: 16.500000
Bucket 5 Min INFO: 12
 ~~~ 
Bucket 6 Max INFO: 22
Bucket 6 Average INFO: 22.000000
Bucket 6 Min INFO: 22
 ~~~ 
Bucket 7 Max INFO: 0
Bucket 7 Average INFO: 0.000000
Bucket 7 Min INFO: 0
 ~~~ 
Bucket 8 Max INFO: 24
Bucket 8 Average INFO: 20.000000
Bucket 8 Min INFO: 16
 ~~~ 
Bucket 9 Max INFO: 0
Bucket 9 Average INFO: 0.000000
Bucket 9 Min INFO: 0
 ~~~ 
Average blocks per bucket: 1.100000
Buckets with overflow blocks: 3
Bucket 1 Overflow Blocks: 2
Bucket 5 Overflow Blocks: 1
Bucket 8 Overflow Blocks: 1
```

# How to run the code
There are numerous ways to run the code. Either make the build and run it or write the command to make it and run it.

## Make Build
To make the build, you have to write `make <Algorithm>` and the Algorithm has to be either hp, ht, or sht. In the folder build, you will find the *<Algorithm>_main*, which you can execute.
For example: ***make sht***

## Make Run
If you want to build and run the program immediately,n you have to write `make <Algorithm>-run` and the Algorithm has to be either hp or ht or sht.
For example: ***make sht-run***

## Make Clean
The `make clean` will delete all the .o .d files and also the data.db and index.db files

# Files - Description
There are 7 different code files: hp_file, hp_main, ht_table, ht_main, sht_table, sht_main, record, and bf_main.
Also, we have the 5 header files for the functions: bf, hp_file, ht_table, sht_table, and record.

### hp_main 
    The main creates the file, opens it, inserts some random records, prints all the records with a specific id, and closes the file.

### hp_file
    It has the functions of heap implementation. The heap has the hp_info, which stores the id file description, max records in the first block and the max records per block. It also has the hp_block_info which stores the count record and the following block id. 

    In the create, we create the first hp_block_info and the hp_info because we create the first block.

    In the open, we take the hp_info

    In the insert we:
        1. take the record
        2. find the last block
        3. check if there is room
            - if there is room, then insert it there
            - if there is not, then create a new block, insert in the new block and link the previous block with the new block
            
    ***Important: The insert starts to insert from the first block that contains the hp_info, the hp_block_info, to save up memory space***

    We go for all the blocks to get all entries and check if we see the record. Because we want to print all the records with the specific id, we pass through all the blocks and records.

    In the close, we close the file and free the hp_info. 
    

### ht_main 
    The main creates the file, opens it, inserts some random records, prints all the records with a specific id, closes the file, and finds the statics of the hash table.

    The hash statistics function prints 
        1. the count of blocks for the data.db file, 
        2. the minimum, the average and the maximum count of records for every bucket of this file,
        3. the average number of blocks for every bucket
        4. the amount of buckets with overflowing blocks and the amount of overflowing blocks for every bucket

### ht_table
    It has the functions of primary hash table implementation. 
    
    The hash table has the ht_info, which stores the id file description, the count of buckets of this hash table, the posistion of the hash table, the max records in the first block of the first bucket and the max records per block. 
    
    It also has the ht_block_info which stores the count record and the following block id. 

    In the create function, we create the first ht_block_info and the hp_info because we create the first block.

    In the open funtion, we take the ht_info

    In the insert we:
        1. take the record
        2. hash the record id in order to find the last block of the appropriate bucket to put it
        3. check if there is room
            - if there is room, then insert it there
            - if there is not, then create a new block, insert in the new block and link the previous block with the new block
            
    ***Important: The insert starts to insert from the first block of the first bucket that contains the ht_info, the ht_block_info, to save up memory space***

    In the get all entries function, we hash the value with the hash function we created in order to find the right bucket. Then, we find this bucket's blocks and we check if they contain the record with the given value. If yes, we print it.

    In the close function, we close the file and free the ht_info. 

### sht_main 
    The main creates the file, then opens it, then inserts some random records in the hash table and then in the secondary hash table and then prints all the records and tries to find a specific name, then closes the file and finds the statics of the secondary hash table.

    The hash statistics function prints 
        1. the count of blocks for the index.db file, 
        2. the minimum, the average and the maximum count of info (name,blockId) for every bucket of the secondary hash table of this file,
        3. the average number of blocks for every bucket
        4. the amount of buckets with overflowing blocks and the amount of overflowing blocks for every bucket

### sht_table
    It has the functions of secondary hash table implementation.

    We created a new struct: in the SHT_node_info we store the name of one record and the id of the ht hashtable block in which the whole record is stored.

    The sht has the sht_info, which stores the id file description, the count of buckets of this hash table, the posistion of the hash table, the max records in the first block of the first bucket and the max records per block. 
    
    It also has the sht_block_info which stores the count record and the following block id. 

    In the create, we create the first sht_block_info and the sht_info because we create the first block.

    In the open, we take the sht_info

    In the insert we:
        1. take the record
        2. find the last block
        3. check if there is room
            - if there is room, then insert it there
            - if there is not, then create a new block, insert in the new block and link the previous block with the new block
            
    ***Important: The insert starts to insert from the first block that contains the sht_info, the sht_block_info, to save up memory space***

   In the get all entries function, we hash the name we were given with the hash function we created in order to find the right bucket. Then, we find this bucket's blocks and we check if they contain a sht_node_info with the given name. If yes, we reach the ht hastable block with the id which is crossed with this name in the sht_node_info. Then, we search the records in the id block and if we find a record with the given name, we print it, else we print that an error occured and we return -1.

    In the close, we close the file and free the sht_info.

### bf_main
    It is an example of how the blocks can handle.

### record
    It has essential functions for the record files.
