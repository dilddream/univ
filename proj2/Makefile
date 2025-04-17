CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = shellex

all: $(PROGS)

shellex: main.c shellex.c csapp.c builtin_command.c pipeline.c jobs.c

clean:
	rm -rf *~ $(PROGS)

