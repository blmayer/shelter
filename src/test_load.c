#include "defs.h"
#include "record.h"
#include "handler.h"
#include "methods.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "map.h"

map addrs;

int main(void) {
	init(&addrs);
	unsigned char rec[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};
	unsigned char *res = malloc(1);

	assert(dump(rec));
	puts("dump(rec) - PASSED");

	assert(load("abc", res));
	puts("load(abc, res) - PASSED");
	
	for (size_t i = 0; i < reclen(rec); i++) {
		assert(rec[i] == res[i]);
	}
	puts("res == rec - PASSED");
}

