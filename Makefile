
# Specify the compiler
CC = g++

#CCOPTS = -ansi -pedantic -Wall -g
CCOPTS = -g -Wall
LIBS = -pthread

# Make the source
all:    router host getIfAddr

getIfAddr : getIfAddr.cpp
	$(CC) getIfAddr.cpp -o getIfAddr

common.o : common.h common.cpp 
	$(CC) $(CCOPTS) -c common.cpp

host:  host.cpp common.o  
	$(CC) $(CCOPTS) $(LIBS) common.o host.cpp -o host

router: router.cpp common.o
	$(CC) $(CCOPTS) $(LIBS) common.o router.cpp -o router

clean :
	rm -f common.o router host getIfAddr
