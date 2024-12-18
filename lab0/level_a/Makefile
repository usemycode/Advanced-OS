# Makefile for Level A: Single Producer and Single Consumer

CC = gcc
CFLAGS = -Wall -std=c99 -O2
TARGET = level_a
SRC = level_a.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -pthread

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
