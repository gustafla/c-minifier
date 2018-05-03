TARGET=minify
CC=gcc
CFLAGS=-Os -g -DSTB_C_LEXER_IMPLEMENTATION
SOURCES=main.c
OBJS=$(patsubst %.c, %.o, $(SOURCES))

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm -f $(TARGET)
	find . -name "*.o" -delete
