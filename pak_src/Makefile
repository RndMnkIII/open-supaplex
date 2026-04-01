CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
SRC_DIR = ../src

PAK_TOOL = pak_tool

.PHONY: all clean

all: $(PAK_TOOL)

$(PAK_TOOL): pak_tool_v2.c $(SRC_DIR)/crc32.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

clean:
	rm -f $(PAK_TOOL)
