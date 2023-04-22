PROGRAMS = ttts ttt protocol
CFLAGS = -g -std=c99 -Wall -Wvla -Werror -fsanitize=address,undefined -pthread

all: $(PROGRAMS)

ttts: ttts.c
	$(CC) $(CFLAGS) $< -o $@

ttt: ttt.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(PROGRAMS) *.o *.a *.dylib *.dSYM