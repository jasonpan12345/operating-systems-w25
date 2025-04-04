CC=gcc
#CFLAGS=-g -O0 #-DNDEBUG
CFLAGS=-DNDEBUG	
FMT=indent

mysh: shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -D MEM_SIZE=$(framesize) -D VAR_SIZE=$(varmemsize) -c shell.c interpreter.c shellmemory.c pcb.c queue.c schedule_policy.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o pcb.o queue.o schedule_policy.o

style: shell.c shell.h interpreter.c interpreter.h shellmemory.c shellmemory.h pcb.c pcb.h queue.c queue.h schedule_policy.c schedule_policy.h
	$(FMT) $?

clean: 
	rm mysh; rm *.o
