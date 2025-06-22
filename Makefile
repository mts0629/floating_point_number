CC := gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -I$(SRCDIR)

SRCDIR := src
TESTDIR := test
OBJDIR = $(BLDDIR)/obj
BLDDIR := build

SRCS = $(wildcard $(SRCDIR)/*.c)
TESTS = $(wildcard $(TESTDIR)/*.c)
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
TARGETS = $(addprefix $(BLDDIR)/, $(basename $(wildcard $(TESTDIR)/*.c)))

DEPS = $(OBJS:.o=.d)
DEPS += $(addsuffix .d, $(subst $(BLDDIR), $(OBJDIR), $(TARGETS)))

.PHONY: all debug format clean

.PRECIOUS: $(OBJDIR)/%.o

all: format $(TARGETS)

debug: CFLAGS += -g -O0
debug: $(TARGETS)

-include $(DEPS)

format:
	@clang-format -i ./src/* ./test/*

$(BLDDIR)/%: $(OBJDIR)/%.o $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -MMD -MP $< -o $@

clean:
	$(RM) -r $(BLDDIR)
