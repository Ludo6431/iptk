EXEC = exec
SRCDIRS = . gtkviewer tools process video
INCDIRS = . gtkviewer tools process video
LIBS = -lm -lrt

# --

CFILES = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS = $(CFILES:.c=.o)
INCPARAMS = $(foreach dir,$(INCDIRS),-I$(CURDIR)/$(dir))

CC = gcc
CFLAGS = -Wall -g $(INCPARAMS) `pkg-config gtk+-2.0 libv4lconvert --cflags` -DDEBUG
#-O2
LDFLAGS = $(LIBS) `pkg-config gtk+-2.0 libv4lconvert --libs`

# debug
#CFLAGS += -DFROM_FILE
#CFLAGS += -DDEBUG_HEDGE
#CFLAGS += -DDEBUG_VEDGE
#CFLAGS += -DOLD_UNDIS

.PHONY:all clean

all:$(EXEC)

clean:
	@echo "RM"
	@rm $(EXEC) $(OBJS)

%.o:%.c
	@echo "CC -o $@"
	@$(CC) $(CFLAGS) -c $< -o $@

$(EXEC):$(OBJS)
	@echo "LD -o $@"
	@$(CC) -o $@ $^ $(LDFLAGS)

