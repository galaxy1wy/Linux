CC = gcc
CFLAGS = -Wall -O2 -pthread
SRC = ./src/logger.c ./src/log_buffer.c ./src/crash_recovery.c ./src/disk_writer.c
OBJ = $(SRC:.c=.o)
TARGET = test/main

all: $(TARGET)

$(TARGET): $(OBJ) test/main.c
	$(CC) $(CFLAGS) -o $@ $^
	@rm -f $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) log_buffer.mmap persisted_log.txt
