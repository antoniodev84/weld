CC      ?= cc
AR      ?= ar
CFLAGS  ?= -O2 -Wall -Wextra -std=c11 -fPIC
PREFIX  ?= /usr/local

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDLIBS += -ldl
endif

LIB  := libweld.a
OBJ  := weld.o weld_utils.o

.PHONY: all clean install uninstall test

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

weld.o: weld.c weld.h
	$(CC) $(CFLAGS) -c $< -o $@

weld_utils.o: weld_utils.c weld_utils.h weld.h
	$(CC) $(CFLAGS) -c $< -o $@

install: $(LIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 644 $(LIB)         $(DESTDIR)$(PREFIX)/lib/
	install -m 644 weld.h         $(DESTDIR)$(PREFIX)/include/
	install -m 644 weld_utils.h   $(DESTDIR)$(PREFIX)/include/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/$(LIB)
	rm -f $(DESTDIR)$(PREFIX)/include/weld.h
	rm -f $(DESTDIR)$(PREFIX)/include/weld_utils.h

test: $(LIB)
	$(MAKE) -C tests
examples: $(LIB)
	$(MAKE) -C examples/app

clean:
	rm -f $(OBJ) $(LIB)
	$(MAKE) -C tests clean
	$(MAKE) -C examples/app clean
