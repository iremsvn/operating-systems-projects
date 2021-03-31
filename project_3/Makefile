all: libsmemlib.a  create_memory destroy_memory app

libsmemlib.a:  smemlib.c
	gcc -Wall -c smemlib.c
	ar -cvq libsmemlib.a smemlib.o
	ranlib libsmemlib.a

app: app.c
	gcc -Wall -o app app.c -L. -lsmemlib -lrt -lpthread -lm

create_memory: create_memory.c
	gcc -Wall -o create_memory create_memory.c -L. -lsmemlib -lrt -lpthread -lm

destroy_memory: destroy_memory.c
	gcc -Wall -o destroy_memory destroy_memory.c -L. -lsmemlib -lrt -lpthread -lm

clean: 
	rm -fr *.o *.a *~ a.out  smemlib.o smemlib.a libsmemlib.a  create_memory destroy_memory app
