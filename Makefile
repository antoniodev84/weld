CC      ?= cc
AR      ?= ar
CFLAGS  ?= -O2 -Wall -Wextra -std=c11 -fPIC
PREFIX  ?= /usr/local

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDLIBS += -ldl
endif

LIB  := libweld.a
OBJ  := weld.o

.PHONY: all clean install uninstall

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

weld.o: weld.c weld.h
	$(CC) $(CFLAGS) -c $< -o $@

install: $(LIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 644 $(LIB)  $(DESTDIR)$(PREFIX)/lib/
	install -m 644 weld.h  $(DESTDIR)$(PREFIX)/include/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/$(LIB)
	rm -f $(DESTDIR)$(PREFIX)/include/weld.h

clean:
	rm -f $(OBJ) $(LIB)
