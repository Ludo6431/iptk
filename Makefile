EXEC := exec
SRCDIRS := . gtkviewer tools process
INCDIRS := . gtkviewer tools process
LIBS := -lm

# --

CFILES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS := $(CFILES:.c=.o)
INCPARAMS := $(foreach dir,$(INCDIRS),-I$(CURDIR)/$(dir))

CC := gcc
CFLAGS := -Wall -g $(INCPARAMS) `pkg-config gtk+-2.0 libv4lconvert --cflags` -DDEBUG
#-O2
LDFLAGS := $(LIBS) `pkg-config gtk+-2.0 libv4lconvert --libs`

# debug
#CFLAGS += -DFROM_FILE
#CFLAGS += -DDEBUG_HEDGE
#CFLAGS += -DDEBUG_VEDGE
#CFLAGS += -DOLD_UNDIS

.PHONY:all clean

all:$(EXEC)

clean:
	rm $(EXEC) $(OBJS)

%.o:%.c
	@echo -e "\x1b[32m"$@"...\x1b[0m"
	@echo -ne "\x1b[33m"
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo -ne "\x1b[0m"

$(EXEC):$(OBJS)
	@echo -e "\x1b[32m"$@"...\x1b[0m"
	@echo -ne "\x1b[33m"
	@$(CC) $(LDFLAGS) -o $@ $(OBJS)
	@echo -ne "\x1b[0m"

