# IMPLEMENTATION OF DATABASE SYSTEM

authors: 
- Despotidou Kalliopi Christina   (sdi2000045)
- Lionis Emmanouil Georgios(Akis) (sdi1900103)

# Summary
This project takes some Records and then saves them in data.db file stores with a heap or a hash table. Also, you can make a secondary hash table that saves the data in index.db. This way, we allocate blocks and data, and then someone can take the data from data.db or index.db and print all the entries. Also we have made some Statistics for some variations.

# Report
There is a report file named "Report.md" where you can find our results and comments.

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