
BIN ?= ghi
PREFIX ?= /usr/local
SRC = ghi.c
DEPS = $(wildcard deps/*/*.c)
OBJS = ghi.o $(DEPS:.c=.o)
CFLAGS = -std=c99 -Wall -Wextra -Ideps
CFLAGS += -DGHI_VERSION=\"0.0.0\" -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lcurl

$(BIN): $(OBJS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

install: $(BIN)
	cp -f $(BIN) $(PREFIX)/bin/$(BIN)

uninstall:
	rm -f $(PREFIX)/bin/$(BIN)

clean:
	rm -f $(OBJS) $(BIN)

.PHONY: install uninstall clean
