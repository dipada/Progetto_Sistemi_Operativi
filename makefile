# finale: dirtest/test.o dir2/stampa.o
#  gcc -o finale dirtest/test.o dir2/stampa.o
# 
# test.o: dirtest/test.c
#  gcc -c dirtest/test.c
# 
# stampa.o: dir2/stampa.c
# gcc -c dir2/stampa.c

start_taxicab: master/master.o master/master_function.o
	gcc -o start_taxicab master/master.o master/master_function.o

master.o: master/master.c
	gcc -c -std=89 -pedantic -Wall master/master.c

master_function.o: master/master_function.c
	gcc -c -std=89 -pedantic -Wall master/master_function.c

clean:
	rm -f ./master/*.o ./mappa/*.o