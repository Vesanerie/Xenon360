CC      = clang
CFLAGS  = -O2 -Wall -Wextra -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lusb-1.0 \
          -framework ApplicationServices -framework CoreFoundation

all: xenon360

xenon360: xenon360.c vhid.c vhid.h
	$(CC) $(CFLAGS) xenon360.c vhid.c -o $@ $(LDFLAGS) -framework IOKit

app: xenon360
	./build_app.sh

run-app: app
	open Xenon360.app

run: xenon360
	./xenon360

verbose: xenon360
	./xenon360 -v

test: test_wireless.c xenon360.c vhid.c vhid.h
	$(CC) $(CFLAGS) -Wno-unused-function -DXENON360_NO_MAIN test_wireless.c vhid.c -o test_wireless $(LDFLAGS) -framework IOKit
	./test_wireless

clean:
	rm -f xenon360 test_wireless app/Xenon360
	rm -rf Xenon360.app

.PHONY: all run verbose clean app run-app test
