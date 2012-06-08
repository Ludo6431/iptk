EXEC := code
SRCDIRS := . bmpencoder gtkviewer generic process
INCDIRS := . bmpencoder gtkviewer generic process
LIBS := -lm

# --

CFILES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS := $(CFILES:.c=.o)
INCPARAMS := $(foreach dir,$(INCDIRS),-I$(CURDIR)/$(dir))

CC := gcc
CFLAGS := -Wall -g $(INCPARAMS) `pkg-config gtk+-2.0 libv4lconvert --cflags` -DDEBUG -O2
# -DOLD_UNDIS
LDFLAGS := $(LIBS) `pkg-config gtk+-2.0 libv4lconvert --libs`

.PHONY:all clean

all:$(EXEC)

clean:
	rm $(EXEC) $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

