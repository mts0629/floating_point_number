CC := gcc
TARGET := parse_fp

.PHONY: clean

parse_fp: parse_fp.c
	$(CC) -Wall -Wextra -Wpedantic -std=c99 $< -o $@

clean:
	$(RM) $(TARGET)
