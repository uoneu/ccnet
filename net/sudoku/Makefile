DIR_INC = ./include
BASE_SRC_DIR = ../../base
NET_SRC_DIR = ..
DIR_OBJ = ./
DIR_BIN = ./bin
LIB= -lpthread  


CC = g++ -std=c++11 

BASE_SRC = $(wildcard ${BASE_SRC_DIR}/*.cc)
NET_SRC = $(wildcard ${NET_SRC_DIR}/*.cc)
OBJ = $(wildcard ${DIR_OBJ}/*.o)



server: $(OBJ)
	$(CC) sudoku.cc server_threadpool.cc $^ -o s.out $(LIB)

batch: $(OBJ)
	$(CC) batch.cc $^ -o c.out $(LIB)

all:
	$(CC) -c $(BASE_SRC) $(NET_SRC)


.PHONY : clean  # clean是伪目标
clean:
	rm  -f *.o  *.out
