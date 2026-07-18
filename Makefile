.PHONY: all clean test debug install lib

PREFIX ?= /usr/local
CFLAGS ?= -g -O0 -Wall -Wextra
OBJS = src/record.o src/methods.o src/map.o src/alloc.o
SERVER_OBJS = $(OBJS) src/handler.o src/main.o

all: db lib

db: $(SERVER_OBJS)
	[ -e bin ] || mkdir bin
	$(CC) $(CFLAGS) $^ -o bin/$@

# Shared client library
lib: bin/libshelter.so

bin/libshelter.so: src/client.c src/client.h src/defs.h
	[ -e bin ] || mkdir bin
	$(CC) $(CFLAGS) -fPIC -shared src/client.c -o $@

release:
	$(MAKE) CFLAGS="-Ofast -static" db

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) src/*.o src/test/*.o
	$(RM) bin/*

test: test_main test_addlink test_alloc

test_main: $(OBJS) src/test/test_main.c
	[ -e bin ] || mkdir bin
	$(CC) $(CFLAGS) src/test/test_main.c $(OBJS) -o bin/$@
	./bin/test_main

test_addlink: $(OBJS) src/test/test_addlink.c
	[ -e bin ] || mkdir bin
	$(CC) $(CFLAGS) src/test/test_addlink.c $(OBJS) -o bin/$@
	./bin/test_addlink

test_dump: $(OBJS) src/test/test_dump.c
	[ -e bin ] || mkdir bin
	$(CC) $(CFLAGS) src/test/test_dump.c $(OBJS) -o bin/$@

test_load: $(OBJS) src/test/test_load.c
	[ -e bin ] || mkdir bin
	$(CC) $(CFLAGS) src/test/test_load.c $(OBJS) -o bin/$@

test_alloc: src/test/test_alloc.c src/alloc.o
	[ -e bin ] || mkdir bin
	$(CC) $(CFLAGS) $^ -o bin/$@
	./bin/$@

debug:
	$(MAKE) CFLAGS="$(CFLAGS) -fsanitize=address" db

install: db lib
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 755 bin/db $(DESTDIR)$(PREFIX)/bin/shelter
	install -m 755 bin/libshelter.so $(DESTDIR)$(PREFIX)/lib/libshelter.so
	install -m 644 src/client.h $(DESTDIR)$(PREFIX)/include/shelter_client.h
	install -m 644 src/defs.h $(DESTDIR)$(PREFIX)/include/shelter_defs.h
