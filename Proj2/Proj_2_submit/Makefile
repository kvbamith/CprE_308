CC=gcc
OBJ = Bank.o appserver.o appserver_coarse.o

all: appserver coarse

appserver: appserver.c Bank.c Bank.h 
	$(CC) appserver.c -o appserver -lpthread

coarse: appserver_coarse.c Bank.c Bank.h 
	$(CC) appserver_coarse.c -o appserver_coarse -lpthread

clean: 
	rm appserver_coarse appserver
