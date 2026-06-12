
EXE = sclipple
OBJS = help.o get_setting.o datetime.o names.o read_list.o git_run.o memo_add.o memo_edit.o main.o

CC = gcc
CFLAGS = -O2 -Wall

%.o : %.c
	$(CC) -c $< $(CFLAGS)

all : $(EXE)

$(EXE) : $(OBJS)
	$(CC) -o $@ $^

.PHONY: clean re debug

clean :
	rm -vf *.o $(EXE)

re : clean all

debug : clean

debug : CFLAGS += -O0 -DDEBUG
debug : clean all


