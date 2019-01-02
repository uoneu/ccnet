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



no_test:  $(OBJ)
	$(CC) no_test.cc $^ -o a.out $(LIB)

buffer_test: $(OBJ)
	$(CC) buffer_test.cc $^ -o a.out $(LIB)

sockets_ops_test: $(OBJ)
	$(CC) sockets_ops_test.cc $^ -o a.out $(LIB)


timerqueue_test: $(OBJ)
	$(CC) timerqueue_test.cc $^ -o a.out $(LIB)

channel_test: $(OBJ)
	$(CC) channel_test.cc $^ -o a.out $(LIB)

timerfd_test: $(OBJ)
	$(CC) timerfd_test.cc $^ -o a.out $(LIB)

acceptor_test: $(OBJ)
	$(CC) acceptor_test.cc $^ -o a.out $(LIB)


eventloop_thread: $(OBJ)
	$(CC) eventloop_thread.cc $^ -o a.out $(LIB)


tcpserver_test: $(OBJ)
	$(CC) tcpserver_test.cc $^ -o a.out $(LIB)

echo_server: $(OBJ)
	$(CC) echo_server.cc $^ -o a.out $(LIB)

echo_client: $(OBJ)
	$(CC) echo_client.cc $^ -o c.out $(LIB)

tcpconn: $(OBJ)
	$(CC) tcpconn.cc $^ -o a.out $(LIB)



all:
	$(CC) -c $(BASE_SRC) $(NET_SRC)


.PHONY : clean  # clean是伪目标
clean:
	rm  -f *.o  *.out
