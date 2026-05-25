CC      = clang
ARCHS   = -arch arm64 -arch x86_64
CFLAGS  = -O2 -Wall -Wextra $(ARCHS) -Ilibs
LDFLAGS = $(ARCHS) libs/libusb-1.0.a \
          -framework ApplicationServices -framework CoreFoundation \
          -framework IOKit -framework Security -framework Foundation \
          -lobjc

INSTALL_DIR = $(HOME)/Library/Application Support/Xenon360

all: xenon360

xenon360: xenon360.c vhid.c vhid.h libs/libusb-1.0.a
	$(CC) $(CFLAGS) xenon360.c vhid.c -o $@ $(LDFLAGS)
	@if [ -d "$(INSTALL_DIR)" ]; then \
		cp $@ "$(INSTALL_DIR)/$@" && \
		echo "  -> refreshed installed copy at $(INSTALL_DIR)/$@"; \
	fi

app: xenon360
	./build_app.sh
	@if [ -d "$(INSTALL_DIR)/Xenon360.app" ]; then \
		rm -rf "$(INSTALL_DIR)/Xenon360.app" && \
		cp -R Xenon360.app "$(INSTALL_DIR)/" && \
		echo "  -> refreshed installed Xenon360.app at $(INSTALL_DIR)/"; \
	fi

run-app: app
	open Xenon360.app

run: xenon360
	./xenon360

verbose: xenon360
	./xenon360 -v

test: test_wireless.c xenon360.c vhid.c vhid.h libs/libusb-1.0.a
	$(CC) $(CFLAGS) -Wno-unused-function -DXENON360_NO_MAIN test_wireless.c vhid.c -o test_wireless $(LDFLAGS)
	./test_wireless

pkg: app
	./build_pkg.sh

release:
	./release.sh

clean:
	rm -f xenon360 test_wireless app/Xenon360 Xenon360.pkg xenon360-component.pkg
	rm -rf Xenon360.app pkg/build

.PHONY: all run verbose clean app run-app test pkg release
