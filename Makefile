CC=gcc
CFLAGS=-ggdb -Wall
LDFLAGS=-lpthread -lm

SRC_DIR=src

SOURCES=$(wildcard $(SRC_DIR)/*.c)
OBJECTS=$(patsubst $(SRC_DIR)/%.c, %.o, $(SOURCES))
EXECUTABLE=taxi_server

ARCHIVE_NAME=chudym

.PHONY: all clean archive

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

archive:
	mkdir -p $(ARCHIVE_NAME)
	cp -r $(SRC_DIR) Makefile $(ARCHIVE_NAME)
	tar czfv $(ARCHIVE_NAME).tar.gz $(ARCHIVE_NAME)
	rm -rf $(ARCHIVE_NAME)
    
.INTERMEDIATE: $(OBJECTS)