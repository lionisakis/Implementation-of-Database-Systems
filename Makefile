IDIR =$(dir $(lastword $(MAKEFILE_LIST)))include
SRC =$(dir $(lastword $(MAKEFILE_LIST)))src
BLD =$(dir $(lastword $(MAKEFILE_LIST)))build
EXA =$(dir $(lastword $(MAKEFILE_LIST)))examples

C=gcc
CFLAGS= -g -Wall -MMD -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ -lbf 

LIBS=-lm -lpthread -lrt -g -Wall -MMD

D= $(SRC)/hp_file.d $(EXA)/hp_main.d $(EXA)/ht_main.d $(EXA)/sht_main.d $(SRC)/sht_table.d $(SRC)/ht_table.d $(SRC)/hp_table.d $(SRC)/record.d

OBJ_HP= $(EXA)/hp_main.o $(SRC)/hp_file.o $(SRC)/record.o
EXEC_HP= $(BLD)/hp_main

OBJ_HT= $(EXA)/ht_main.o $(SRC)/ht_table.o $(SRC)/record.o
EXEC_HT= $(BLD)/ht_main

OBJ_SHT= $(EXA)/sht_main.o $(SRC)/sht_table.o $(SRC)/ht_table.o $(SRC)/record.o
EXEC_SHT= $(BLD)/sht_main

ARGS = 

$(EXEC_HP): $(OBJ_HP)
	@echo " --- Compile hp_main --- ";
	$(C) -o $(EXEC_HP) $(OBJ_HP) $(CFLAGS) $(LIBS)

$(EXEC_HT): $(OBJ_HT)
	@echo " --- Compile ht_main --- ";
	$(C) -o $(EXEC_HT) $(OBJ_HT) $(CFLAGS) $(LIBS)

$(EXEC_SHT): $(OBJ_SHT)
	@echo " --- Compile sht_main --- ";
	$(C) -o $(EXEC_SHT) $(OBJ_SHT) $(CFLAGS) $(LIBS)

hp: $(OBJ_HP)
	@echo " --- Compile hp_main --- ";
	$(C) -o $(EXEC_HP) $(OBJ_HP) $(CFLAGS) $(LIBS)

ht: $(OBJ_HT)
	@echo " --- Compile ht_main --- ";
	$(C) -o $(EXEC_HT) $(OBJ_HT) $(CFLAGS) $(LIBS)
	
sht: $(OBJ_SHT)
	@echo " --- Compile sht_main --- ";
	$(C) -o $(EXEC_SHT) $(OBJ_SHT) $(CFLAGS) $(LIBS)

hp-run: $(EXEC_HP)
	@echo " --- Run hp_main --- ";
	$(EXEC_HP) $(ARGS) 

ht-run: $(EXEC_HT)
	@echo " --- Run ht_main --- ";
	$(EXEC_HT) $(ARGS) 

sht-run: $(EXEC_SHT)
	@echo " --- Run sht_main --- ";
	$(EXEC_SHT) $(ARGS) 

.PHONY: clean

clean:
	rm -f $(OBJ_HP) $(OBJ_HT) $(OBJ_SHT) $(EXEC_SHT) $(EXEC_HT) $(EXEC_HP) $(D) $(OUTPUT) data.db index.db
