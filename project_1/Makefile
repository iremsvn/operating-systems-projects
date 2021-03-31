all: pmc tmc

pmc: pmc.c
	gcc -Wall -o pmc pmc.c -lrt

tmc: tmc.c
	gcc -Wall -o tmc tmc.c -lpthread


clean:
	rm -fr *~ *.o pmc tmc
