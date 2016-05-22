CC=gcc
CFLAGS=-ggdb -Wall
LDFLAGS=-lpthread -lm

SRC_DIR=src
OUT_DIR=build

SOURCES=$(wildcard $(SRC_DIR)/*.c)
OBJECTS=$(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/%.o, $(SOURCES))
EXECUTABLE=taxi_server

ARCHIVE_NAME=chudym

.PHONY: all dirs clean archive

all: dirs $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(OUT_DIR)/$@ $(OBJECTS) $(LDFLAGS)

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

dirs:
	mkdir -p $(OUT_DIR)

clean:
	rm -rf $(OUT_DIR)/*

archive:
	mkdir -p $(ARCHIVE_NAME)
	cp -r $(SRC_DIR) Makefile $(ARCHIVE_NAME)
	tar czfv $(ARCHIVE_NAME).tar.gz $(ARCHIVE_NAME)
	rm -rf $(ARCHIVE_NAME)
    
.INTERMEDIATE: $(OBJECTS)