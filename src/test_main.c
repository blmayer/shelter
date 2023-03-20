#include <stdio.h>
#include <assert.h>
#include <map.h>
#include "defs.h"
#include "handler.h"
#include "methods.h"
#include "record.h"

map addrs;

int main(void) {
	init(&addrs);
	unsigned char rec[] = {type_record, 16, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', 4, 3, 0, 0, 0, 'x', 'y', 'z'};

	puts("starting tests");
	assert(putkey(rec));
	puts("putkey(rec) - PASSED");

	unsigned char *res = fetchkey("abc");
	assert(res != NULL);
	puts("fetchkey(key) != NULL - PASSED");

	for (int i = 0; i < datalen(rec) + typelen(rec); i++) {
		assert(rec[i] == res[i]);
	}
	puts("res == rec - PASSED");

	res = fetchkey("bcx");
	assert(res == NULL);
	puts("fetchkey() == NULL - PASSED");
}

