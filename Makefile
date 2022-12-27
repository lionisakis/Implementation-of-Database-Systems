# hp:
# 	@echo " Compile hp_main ...";
# 	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main.c ./src/record.c ./src/hp_file.c -lbf -o ./build/hp_main -O2

# bf:
# 	@echo " Compile bf_main ...";
# 	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c ./src/record.c -lbf -o ./build/bf_main -O2;

# ht:
# 	@echo " Compile hp_main ...";
# 	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/record.c ./src/ht_table.c -lbf -o ./build/ht_main -O2

# sht:
# 	@echo " Compile hp_main ...";
# 	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sht_main.c ./src/record.c ./src/sht_table.c ./src/ht_table.c -lbf -o ./build/sht_main -O2

IDIR =./$(dir $(lastword $(MAKEFILE_LIST)))include
MDIR =./$(dir $(lastword $(MAKEFILE_LIST)))src/
EXA =./$(dir $(lastword $(MAKEFILE_LIST)))examples/

C=gcc
CFLAGS= -g -Wall -MMD -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ 

LIBS=-lm -lpthread -lrt -g -Wall -MMD -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ -lbf 

OBJ = ./examples/sht_main.o ./src/ht_table.o ./src/sht_table.o ./src/record.o
D= ./examples/sht_main.d ./src/ht_table.d ./src/sht_table.d ./src/record.d
EXEC= ./build/sht_main

ARGS = 

$(EXEC): $(OBJ) 
	$(C) -o $(EXEC) $(OBJ) $(CXXFLAGS) $(LIBS) 

run: $(EXEC)
	./$(EXEC) $(ARGS) 

.PHONY: clean

clean:
	rm -f $(OBJ) $(EXEC) $(D) $(OUTPUT) data.db index.db

valgrind: $(OBJ)
	$(C) -o $(EXEC) $^ $(CXXFLAGS) $(LIBS)
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes ./$(EXEC) $(ARGS) 