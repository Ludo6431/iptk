EXEC := code
SRCDIRS := . bmpencoder tinyjpegdecoder gtkviewer
INCDIRS := . bmpencoder tinyjpegdecoder gtkviewer
LIBS := -lm

# --

CFILES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS := $(CFILES:.c=.o)
INCPARAMS := $(foreach dir,$(INCDIRS),-I$(CURDIR)/$(dir))

CC := gcc
CFLAGS := -Wall -g $(INCPARAMS) `pkg-config gtk+-2.0 --cflags`
LDFLAGS := $(LIBS) `pkg-config gtk+-2.0 --libs`

.PHONY:all clean

all:$(EXEC)

clean:
	rm $(EXEC) $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

