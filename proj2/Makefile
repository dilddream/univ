CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = shellex

all: $(PROGS)

shellex: shellex.c csapp.c builtin_command.c

clean:
	rm -rf *~ $(PROGS)

