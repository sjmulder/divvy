DESTDIR   ?=
PREFIX    ?= /usr/local
MANPREFIX ?= $(PREFIX)/man

CFLAGS += -Wall -Wextra

all: divvy

clean:
	rm -f divvy

install: divvy
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(MANPREFIX)/man1
	install -m755 divvy $(DESTDIR)$(PREFIX)/bin/
	install -m644 divvy.1 $(DESTDIR)$(MANPREFIX)/man1/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/divvy
	rm -f $(DESTDIR)$(MANPREFIX)/man1/divvy.1

.PHONY: all clean install uninstall
