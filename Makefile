CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = file-processor.c main.c checkin.c
OBJS = file-processor.o main.o checkin.o
TARGET = longestwords

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

file-processor.o: file-processor.c
	$(CC) $(CFLAGS) -c file-processor.c


clean:
	rm -f $(OBJS) $(TARGET) 