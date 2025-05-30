CC := gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -I$(SRCDIR)

SRCDIR := src
OBJDIR = $(BLDDIR)/obj
BLDDIR := build

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
TARGETS = $(addprefix $(BLDDIR)/, $(basename $(wildcard *.c)))

DEPS = $(OBJS:.o=.d)
DEPS += $(addsuffix .d, $(subst $(BLDDIR), $(OBJDIR), $(TARGETS)))

.PHONY: all debug format clean

.PRECIOUS: $(OBJDIR)/%.o

all: format $(TARGETS)

debug: CFLAGS += -g -O0
debug: $(TARGETS)

-include $(DEPS)

format:
	@clang-format -i ./src/* ./*.c

$(BLDDIR)/%: $(OBJDIR)/%.o $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -MMD -MP $< -o $@

clean:
	$(RM) -r $(BLDDIR)
