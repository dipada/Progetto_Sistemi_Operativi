CC= gcc
CFLAGS= -c -std=c89 -pedantic -Wall
LINKING= $(CC) -o

all: start_taxicab map
	
start_taxicab: master/master.o master/master_function.o
	$(LINKING) start_taxicab master/master.o master/master_function.o 

master.o: master/master.c master/master_function.c
	$(CC) $(CFLAGS) master/master.c master/master_function.c

map: mappa/map.o mappa/map_operation.o
	$(LINKING) mappa/map mappa/map.o mappa/map_operation.o

map.o: mappa/mappa.c mappa/map_operation.c
	$(CC) $(CFLAGS) mappa/map.c mappa/map_operation.c

clean:
	rm -f ./master/*.o ./mappa/*.o
