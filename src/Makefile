
EXE = sclipple
OBJS = help.o 		 \
	   strutils.o	 \
	   get_setting.o \
	   datetime.o 	 \
	   names.o 		 \
	   edit_list.o 	 \
	   git_run.o 	 \
	   memo_add.o 	 \
	   memo_edit.o 	 \
	   memo_rm.o 	 \
	   memo_mv.o 	 \
	   memo_ls.o 	 \
	   memo_search.o \
	   memo_show.o 	 \
	   main.o

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


