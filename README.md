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

### ht_table
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

### sht_main 
    The main creates the file, then opens it, then inserts some random records in the hash table and then in the secondary hash table and then prints all the records and tries to find a specific name, then closes the file and finds the statics of the secondary hash table.

### sht_table
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

### bf_main
    It is an example of how the blocks can handle.

### record
    It has essential functions for the record files.