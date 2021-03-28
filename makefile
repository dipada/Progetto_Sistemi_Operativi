all: start_taxicab mappa

start_taxicab: master/master.o master/master_function.o 
	gcc -o start_taxicab master/master.o master/master_function.o 

mappa: mappa/mappa.o mappa/map_operation.o
	gcc -o mappa/mappa mappa/mappa.o mappa/map_operation.o

master.o: master/master.c
	gcc -c -std=89 -pedantic -Wall master/master.c

master_function.o: master/master_function.c
	gcc -c -std=89 -pedantic -Wall master/master_function.c

mappa.o: mappa/mappa.c
	gcc -c -std=89 -pedantic -Wall mappa/mappa.c

map_operation.o: mappa/map_operation.c
	gcc -c -std=89 -pedantic -Wall mappa/map_operation.c

clean:
	rm -f ./master/*.o ./mappa/*.o