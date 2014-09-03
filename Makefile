
BIN ?= ghi
PREFIX ?= /usr/local
SRC = ghi.c
DEPS = $(wildcard deps/*/*.c)
GHI_VERSION := `node -pe "require('./package.json').version"`
OBJS = ghi.o $(DEPS:.c=.o)
CFLAGS = -std=c99 -Wall -Wextra -Ideps
CFLAGS += -D_POSIX_C_SOURCE=200809L
CFLAGS += -DGHI_VERSION=\"$(GHI_VERSION)\"
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
