DIR_INC = ./include
DIR_SRC = ..
DIR_OBJ = ./
DIR_BIN = ./bin
LIB= -lpthread  


CC = g++ -std=c++11 

SRC = $(wildcard ${DIR_SRC}/*.cc)
OBJ = $(wildcard ${DIR_OBJ}/*.o)


no_test:  $(OBJ)
	$(CC) no_test.cc $^ -o a.out $(LIB)


atomic_test:
	$(CC) atomic_test.cc -o a.out


string_piece_test:
	$(CC) string_piece_test.cc -o a.out



mutex_test: $(OBJ)
	$(CC)  mutex_test.cc $^  -o a.out $(LIB)



timestamp_test: timestamp.o
	$(CC) timestamp_test.cc $< -o a.out


date_unittest: date.o
	$(CC) date_unittest.cc $< -o a.out


time_zone_test: time_zone.o date.o
	$(CC) time_zone_test.cc $^ -o a.out


singleton_test: $(OBJ)
	$(CC) singleton_test.cc $^ -o a.out $(LIB)


exception_test: $(OBJ)
	$(CC) exception_test.cc $^ -o a.out $(LIB)


thread_test: $(OBJ)
	$(CC) thread_test.cc $^ -o a.out $(LIB)


thread_local_test: $(OBJ)
	$(CC) thread_local_test.cc $^ -o a.out $(LIB)



threadPool_test: $(OBJ)
	$(CC) threadPool_test.cc $^ -o a.out $(LIB)


logging_test: $(OBJ)
	$(CC) logging_test.cc $^ -o a.out $(LIB)


all:
	$(CC) -c $(SRC)
	


.PHONY : clean  # clean是伪目标
clean:
	rm  -f *.o  a.out