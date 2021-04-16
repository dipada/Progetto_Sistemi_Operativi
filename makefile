CC= gcc
CFLAGS= -c -std=c89 -pedantic -Wall
LINKING= $(CC) -o

all: start_taxicab map source
	
start_taxicab: master/master.o master/master_operation.o
	$(LINKING) start_taxicab master/master.o master/master_operation.o 

master.o: master/master.c master/master_operation.c
	$(CC) $(CFLAGS) master/master.c master/master_operation.c

map: mappa/map.o mappa/map_operation.o
	$(LINKING) mappa/map mappa/map.o mappa/map_operation.o

map.o: mappa/mappa.c mappa/map_operation.c
	$(CC) $(CFLAGS) mappa/map.c mappa/map_operation.c

taxi: taxi/taxi.o taxi/taxi_operation.o
	$(LINKING) taxi/taxi taxi/taxi.o taxi/taxi_operation.o

taxi.o: taxi/taxi.c taxi/taxi_operation.c
	$(CC) $(CFLAGS) taxi/taxi.c taxi/taxi_operation.c

source: source/source.o source/source_operation.o
	$(LINKING) source/source source/source.o source/source_operation.o

source.o: source/source.c source/source_operation.c
	$(CC) $(CFLAGS) source/source.c source/source_operation.c

clean:
	rm -f ./master/*.o ./mappa/*.o ./taxi/*.o ./source/*.o start_taxicab ./source/source mappa/map taxi/taxi
