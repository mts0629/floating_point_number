CC := gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -I$(SRCDIR)

SRCDIR := src
TESTDIR := test
BUILDDIR := build

SRCS = $(wildcard $(SRCDIR)/*.c)
TESTS = $(wildcard $(TESTDIR)/*.c)
OBJS = $(addprefix $(BUILDDIR)/, $(SRCS:.c=.o))
TARGETS = $(patsubst $(TESTDIR)/%, $(BUILDDIR)/bin/%, $(TESTS:.c=))

DEPS = $(OBJS:.o=.d)
DEPS += $(addprefix $(BUILDDIR)/, $(TESTS:.c=.d))

.PHONY: all debug format test clean

.PRECIOUS: $(BUILDDIR)/%.o

all: format $(TARGETS)

debug: CFLAGS += -g -O0
debug: $(TARGETS)

-include $(DEPS)

format:
	@clang-format -i ./src/* ./test/*

test: $(TARGETS)
	@for target in $(TARGETS); do echo "$${target}"; ./$${target}; done

$(BUILDDIR)/bin/%: $(TESTDIR)/%.c $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -MMD -MP $< -o $@

clean:
	$(RM) -r $(BUILDDIR)
