CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c2x -O2 -g
CFLAGS += -D_FORTIFY_SOURCE=3
CFLAGS += -fstack-protector-strong
CFLAGS += -fPIE
LDFLAGS = -pie -z relro -z now

SRCDIR = src
INCDIR = include
BUILDDIR = build
TESTDIR = tests
CONNECTORDIR = connector

SRCS = $(wildcard $(SRCDIR)/*.c) main.c
OBJS = $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

TARGET = frailbox
SELF_TEST_TARGET = $(BUILDDIR)/self_test
CONNECTOR_TEST_TARGET = $(BUILDDIR)/test_connector
SELF_TEST_SRCS = $(TESTDIR)/self_test.c $(SRCDIR)/arena.c $(SRCDIR)/logger.c $(SRCDIR)/sandbox.c $(CONNECTORDIR)/api.c $(CONNECTORDIR)/protocol.c
CONNECTOR_TEST_SRCS = $(TESTDIR)/test_connector.c $(CONNECTORDIR)/api.c $(CONNECTORDIR)/protocol.c

.PHONY: all clean self-test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -I$(INCDIR) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCDIR) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

distclean: clean
	rm -rf *.o *.d

test: $(TARGET)
	./$(TARGET) --sandbox-type seccomp --memory-limit 64 --verbose

$(SELF_TEST_TARGET): $(SELF_TEST_SRCS)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(CONNECTORDIR) $^ -o $@ $(LDFLAGS) -pthread

$(CONNECTOR_TEST_TARGET): $(CONNECTOR_TEST_SRCS)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(CONNECTORDIR) $^ -o $@ $(LDFLAGS) -pthread

self-test: $(SELF_TEST_TARGET) $(CONNECTOR_TEST_TARGET)
	@passed=0; failed=0; skipped=0; \
	echo "frailbox self-test suite"; \
	echo "======================="; \
	echo "running smoke checks"; \
	if ./$(SELF_TEST_TARGET); then passed=$$((passed + 1)); else failed=$$((failed + 1)); fi; \
	echo ""; \
	echo "running connector tests"; \
	if ./$(CONNECTOR_TEST_TARGET); then passed=$$((passed + 1)); else failed=$$((failed + 1)); fi; \
	echo ""; \
	echo "self-test summary: passed=$$passed failed=$$failed skipped=$$skipped total=$$((passed + failed + skipped))"; \
	test $$failed -eq 0

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

.PHONY: all clean distclean test self-test valgrind
