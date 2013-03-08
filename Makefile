MAIN=yaca
#CFLAGS=-W -Wall -ansi -pedantic

all:	clean code.o
	gcc ${CFLAGS} -o ${MAIN} code.o ${MAIN}.c

code.o:
	gcc -c code.c

clean:
	rm -f *.o *.*~ *~ ${MAIN}