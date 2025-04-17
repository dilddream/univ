CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

SRCS = main.c csapp.c command_handle.c execute.c jobs.c
PROGS = myshell

all: $(PROGS)

$(PROGS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(PROGS) $(LDLIBS)

clean:
	rm -rf *~ $(PROGS)

