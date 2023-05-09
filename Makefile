CC			=  gcc
CFLAGS		+= -pedantic -Wall -g
INCLUDES	= -I ./includes
TARGETS		= farm
OBJS        = ./source/main.o ./source/utils.o ./source/bst.o ./source/ThreadsPool.o ./source/collector.o ./source/masterWorker.o

.PHONY: all clear cleanExe test testSig testClean testAll
.SUFFIXES: .c .h

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

all: $(TARGETS)



testAll: 	test testSig

test: 		generafile
	./test.sh

testSig:	generafile
	./testSigUsr.sh

generafile: ./source/generafile.c
	gcc -std=c99 -o $@ $^

farm : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

./source/generafile.o : ./source/generafile.c

./source/masterWorker.o : ./source/masterWorker.c

./source/collector.o : ./source/collector.c

./source/ThreadsPool.o : ./source/ThreadsPool.c

./source/main.o : ./source/main.c

./source/utils.o : ./source/utils.c

./source/bst.o :./source/bst.c

cleanAll: clear cleanExe

clear :
	-rm ./source/*.o
	-rm *.sck

cleanExe :
	\rm -f *.dat
	\rm -r testdir
	\rm -f generafile
	\rm -f farm
