CC = gcc
CFLAGS = -g -finstrument-functions -Wall -Werror
CFLAGS2 = -g  -Wall -Werror

all: crumble

crumble.o: crumble.c
	$(CC) $(CFLAGS) -o crumble.o -c crumble.c

ptrace.o: ptrace.c
	$(CC) $(CFLAGS2) -o ptrace.o -c ptrace.c

lib.o: lib.c
	$(CC) $(CFLAGS) -o lib.o -c lib.c

crumble: crumble.o ptrace.o lib.o
	$(CC) crumble.o ptrace.o lib.o -o crumble -lpthread -rdynamic

run:
	touch TRACE ;
	./crumble
	tail -f TRACE
	rm -f TRACE

clean:
	rm -f crumble TRACE *.o
