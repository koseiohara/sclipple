
EXE = nt
OBJS = git_run.o main.o

CC = gcc
CFLAGS = -O2 -Wall

%.o : %.c
	$(CC) -c $< $(CFLAGS)

all : $(EXE)

$(EXE) : $(OBJS)
	$(CC) -o $@ $^

.PHONY: clean re

clean :
	rm -vf *.o $(EXE)

re : clean all


