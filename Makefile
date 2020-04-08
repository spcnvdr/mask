# A simple Makefile, to build run: make all 

CC	= gcc
#compiler flags here
CFLAGS = -O3 -Wall -Wextra

#linker flags here
LFLAGS = -Wall

SOURCES	:= $(wildcard /*.c)
INCLUDES := $(wildcard /*.h))
OBJECTS	:= mask.o


.PHONY: all clean remove
all: mask

mask: $(OBJECTS)
	@$(CC) -o $@ $(LFLAGS) mask.o

$(OBJECTS): ./%.o : ./%.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$ rm -f $(OBJECTS)

remove: clean
	@$ rm -f mask
