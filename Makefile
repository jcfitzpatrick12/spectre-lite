CC=gcc
CFLAGS=-Iinclude -lm -lfftw3
SRC=$(wildcard src/*.c)
TARGET=spectrel

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET)
