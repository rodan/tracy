
CC      = gcc
CFLAGS  = -g -Wall -O2
#LDFLAGS = 
PROGS   = read_mem

read_mem_OBJS    = read_mem.o
read_mem_EXTRA   = read_mem.h

SRCS   = read_mem.c
EXTRA  = ../AUTHORS ../COPYING ../ChangeLog \
			../INSTALL ../NEWS ../README \
			Makefile

all: ${PROGS} tags

.c.o:
	$(CC) -c $(CFLAGS) $*.c

read_mem : $(read_mem_OBJS) $(read_mem_EXTRA)
	$(CC) -o read_mem $(read_mem_OBJS) $(LIBS) $(LDFLAGS)

tags : $(read_mem_OBJS) $(read_mem_EXTRA)
	exuberant-ctags -h ".h.c" *

clean :
	rm -f *.o *.sibling *.combine $(PROGS)

strip : $(read_mem_OBJS) $(read_mem_EXTRA)
	strip ${PROGS}


