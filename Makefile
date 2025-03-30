CC := gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -I$(INCDIR)

SRCDIR := .
INCDIR := .
BLDDIR := ./build

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(addprefix $(BLDDIR)/, $(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

TARGET = $(BLDDIR)/parse_fp32

.PHONY: all mkd clean

all: $(TARGET)

-include $(DEPS)

mkd:
	@mkdir -p $(BLDDIR)/$(SRCDIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BLDDIR)/%.o: $(SRCDIR)/%.c mkd
	$(CC) $(CFLAGS) -c -MMD -MP $< -o $@

clean:
	$(RM) $(BLDDIR)
