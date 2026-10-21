CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11 -fPIC
WELD    := ..

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDLIBS += -ldl
endif

MODULES := modules/a.so modules/b.so modules/c.so modules/bad.so \
           modules/cycle_x.so modules/cycle_y.so

.PHONY: all run clean

all: $(MODULES) test_runner
	./test_runner

test_runner: main.c $(WELD)/libweld.a $(MODULES)
	$(CC) $(CFLAGS) -I$(WELD) -o $@ main.c $(WELD)/libweld.a $(LDLIBS)

modules/%.so: modules/%.c $(WELD)/weld.h
	$(CC) $(CFLAGS) -I$(WELD) -shared -o $@ $<

clean:
	rm -f test_runner $(MODULES) /tmp/weld_list.txt /tmp/weld_tree.txt
