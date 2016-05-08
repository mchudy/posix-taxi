CC=gcc
CFLAGS=-ggdb -Wall
LFLAGS=-lpthread
FILE=
SRC_DIR=src
OUT_DIR=build

all:
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $(OUT_DIR)/taxi_server $(SRC_DIR)/common.c $(SRC_DIR)/taxi_server.c $(LFLAGS)

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)/*
