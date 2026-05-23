CC      = clang
CFLAGS  = -O2 -Wall -Wextra -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lusb-1.0 \
          -framework ApplicationServices -framework CoreFoundation

all: xenon360

xenon360: xenon360.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: xenon360
	./xenon360

verbose: xenon360
	./xenon360 -v

clean:
	rm -f xenon360

.PHONY: all run verbose clean
