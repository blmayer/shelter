.PHONY: main clean test debug test_hash
CFLAGS=-g -O0 -Wall -Wextra

main: $(patsubst %.c,%.o,$(wildcard src/*.c))
	[ -e bin ] || mkdir bin
	$(CC) $(CFLAGS) $^ -o bin/$@

release:
	$(MAKE) CFLAGS="-Ofast -static" main

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) src/*.o
	$(RM) bin/*

test: test_hash
	./bin/$<

test_hash: src/test/test_hash.c src/aux.c
	$(CC) $(CFLAGS) $^ -o bin/$@

test_main: src/test_main.c src/record.o src/methods.o src/map.o
	$(CC) $(CFLAGS) $^ -o bin/$@

test_addlink: src/test_addlink.c src/record.o src/methods.o src/map.o
	$(CC) $(CFLAGS) $^ -o bin/$@

test_dump: src/test_dump.c src/record.o src/methods.o src/map.o
	$(CC) $(CFLAGS) $^ -o bin/$@

test_load: src/test_load.c src/record.o src/methods.o src/map.o
	$(CC) $(CFLAGS) $^ -o bin/$@

test_alloc: src/test_alloc.c src/alloc.o
	$(CC) $(CFLAGS) $^ -o bin/$@
	./bin/$@

debug:
	$(MAKE) CFLAGS="$(CFLAGS) -fsanitize=address" main
	
