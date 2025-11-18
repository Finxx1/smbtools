# use these arguments to compile for windows on posix
# CC=x86_64-w64-mingw32-gcc
OBJ = \
	unpack.o \

LDFLAGS = -O2
BUILD_DIR = ../build

unpack: $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $(BUILD_DIR)/unpack

.PHONY: all
all: unpack

.PHONY: clean
clean:
	rm -rf *.o
