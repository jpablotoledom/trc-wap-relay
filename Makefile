# Builds dist/trc-wap-relay.exe for Windows XP and later.
#
#   Linux (cross):   make                      (needs gcc-mingw-w64-i686)
#   Windows (MinGW): make CC=gcc
#
# The subsystem/OS version flags keep the PE header at 5.01 so XP accepts the
# binary; -static avoids depending on MinGW runtime DLLs on the target PC.

CC      ?= i686-w64-mingw32-gcc
ifeq ($(origin CC),default)
CC      := i686-w64-mingw32-gcc
endif

CFLAGS  ?= -O2 -Wall -Wextra
CFLAGS  += -std=gnu99 -D_WIN32_WINNT=0x0501
LDFLAGS += -s -static \
           -Wl,--major-subsystem-version,5,--minor-subsystem-version,1 \
           -Wl,--major-os-version,5,--minor-os-version,1
LDLIBS  += -lws2_32 -liphlpapi

SRC     := $(wildcard src/*.c)
OBJ     := $(patsubst src/%.c,build/%.o,$(SRC))
TARGET  := dist/trc-wap-relay.exe

all: $(TARGET)

$(TARGET): $(OBJ) | dist
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

build/%.o: src/%.c $(wildcard src/*.h) | build
	$(CC) $(CFLAGS) -c -o $@ $<

build dist:
	mkdir -p $@

test: $(TARGET)
	tests/run_wine_tests.sh

clean:
	rm -rf build dist

.PHONY: all test clean
