#include <stdio.h>
#include <assert.h>
#include "map.h"
#include "defs.h"
#include "handler.h"
#include "methods.h"
#include "record.h"

map addrs;

int main(void) {
	init(&addrs);
	unsigned char rec[] = {type_record, 16, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};
	unsigned char rec1[] = {type_record, 16, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'd', type_string, 3, 0, 0, 0, 'z', 'y', 'x'};

	puts("starting tests");
	printrec(rec);
	assert(putkey(rec) == 0);
	puts("putkey(rec) - PASSED");
	printf("map: %p\n", addrs.next);

	unsigned char *res = fetchkey("abc");
	assert(res != NULL);
	puts("fetchkey(abc) != NULL - PASSED");

	for (size_t i = 0; i < datalen(rec) + typelen(rec); i++) {
		assert(rec[i] == res[i]);
	}
	puts("res == rec - PASSED");
	printf("map: %p\n", addrs.next);

	res = fetchkey("bcx");
	assert(res == NULL);
	puts("fetchkey(bcx) == NULL - PASSED");

	assert(putkey(rec1) == 0);
	puts("putkey(rec1) - PASSED");
	printf("map: %p\n", addrs.next);

	res = fetchkey("abc");
	assert(res != NULL);
	puts("fetchkey(abc) != NULL - PASSED");

	for (size_t i = 0; i < datalen(rec) + typelen(rec); i++) {
		assert(rec[i] == res[i]);
	}
	puts("res == rec - PASSED");

	res = fetchkey("abd");
	assert(res != NULL);
	puts("fetchkey(abd) != NULL - PASSED");

	for (size_t i = 0; i < datalen(rec1) + typelen(rec1); i++) {
		assert(rec1[i] == res[i]);
	}
	puts("res == rec1 - PASSED");
}

