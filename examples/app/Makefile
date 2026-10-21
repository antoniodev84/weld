CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11 -fPIC
WELD    := ../..

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDLIBS += -ldl
endif

MODULES := modules/io.so modules/fs.so
APP     := filecli

.PHONY: all clean run modules weld

all: weld $(MODULES) $(APP)

weld:
	$(MAKE) -C $(WELD)

$(APP): main.c $(WELD)/libweld.a
	$(CC) $(CFLAGS) -I$(WELD) -o $@ main.c $(WELD)/libweld.a $(LDLIBS)

modules/%.so: modules/%.c $(WELD)/weld.h
	$(CC) $(CFLAGS) -I$(WELD) -shared -o $@ $<

run: all
	./$(APP) modules
	./$(APP) list .

clean:
	rm -f $(APP) $(MODULES)
	$(MAKE) -C $(WELD) clean
