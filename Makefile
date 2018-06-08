TARGET=minify
CC=gcc
CFLAGS+=-Os -g -DSTB_C_LEXER_IMPLEMENTATION
SOURCES=main.c str_hashmap.c
OBJS=$(patsubst %.c,%.o,$(SOURCES))

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean install

PREFIX=~/.local

install: $(TARGET)
	install -d $(PREFIX)/bin/
	cp $(TARGET) $(PREFIX)/bin/

clean:
	rm -f $(TARGET) $(OBJS)
