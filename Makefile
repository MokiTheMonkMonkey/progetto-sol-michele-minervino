CC			=  gcc
CFLAGS		+= -pedantic -Wall -g
INCLUDES	= -I ./includes
TARGETS		= farm
OBJS        = ./filesC/util.o ./filesC/main.o

.PHONY: all clear
.SUFFIXES: .c .h

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

all: $(TARGETS)

farm : ./filesC/main.o ./filesC/utils.o ./filesC/bst.o ./filesC/ThreadsPool.o ./filesC/collector.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

./filesC/collector.o : ./filesC/collector.c

./filesC/ThreadsPool.o : ./filesC/ThreadsPool.c

./filesC/main.o : ./filesC/main.c

./filesC/utils.o : ./filesC/utils.c

./filesC/bst.o :./filesC/bst.c

clear :
	-rm ./filesC/*.o
	-rm farm
	-rm *.sck