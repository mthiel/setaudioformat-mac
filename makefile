UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Darwin)
    $(error This tool can only be built for MacOS)
endif

CC = clang
COMMON_FLAGS = -framework CoreAudio
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2 -DNDEBUG

TARGET = setAudioFormat
SRC = setAudioFormat.c

all: clean default

default: CFLAGS = $(COMMON_FLAGS) $(RELEASE_FLAGS)
default: $(TARGET)

debug: clean
debug: CFLAGS = $(COMMON_FLAGS) $(DEBUG_FLAGS)
debug: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -f $(TARGET)
	rm -rf $(TARGET).dSYM
