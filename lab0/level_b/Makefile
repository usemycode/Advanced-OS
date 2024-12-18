# Makefile for Level B: Multiple Producers and Multiple Consumers

CC = gcc
CFLAGS = -Wall -std=c99 -O2
TARGET = level_b
SRC = level_b.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -pthread

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
