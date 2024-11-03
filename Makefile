CC = gcc

# Compiler flags
CFLAGS = --std=c89 -Wall -Wextra -pthread -pedantic -Wno-declaration-after-statement

TARGET = mssv

# Source files
SRCS = mssv.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
