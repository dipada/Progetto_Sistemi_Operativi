GCC = gcc -c
FLAGS = -std=c89 -pedantic -Wall
LINKING = gcc -o

all: start_taxicab map

start_taxicab: master/master.o master/master_function.o 
	$(LINKING) start_taxicab master/master.o master/master_function.o 

master.o: master/master.c
	$(GCC) $(FLAGS) master/master.c

master_function.o: master/master_function.c
	$(GCC) $(FLAGS) master/master_function.c

map: mappa/map.o mappa/map_operation.o
	$(LINKING) mappa/map mappa/map.o mappa/map_operation.o

map.o: mappa/mappa.c
	$(GCC) $(FLAGS) mappa/map.c

map_operation.o: mappa/map_operation.c
	$(GCC) $(FLAGS) mappa/map_operation.c

clean:
	rm -f ./master/*.o ./mappa/*.o