#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "map.h"
#include "defs.h"
#include "handler.h"
#include "methods.h"
#include "record.h"

map addrs;

int main(void) {
	init(&addrs);
	unsigned char rect[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};
	unsigned char rest[] = {type_record, 40, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z', type_link, 14, 0, 0, 0, type_key, 2, 0, 0, 0, 't', 'o', type_key, 3, 0, 0, 0, 'd', 'e', 'f'};

	unsigned char *rec = malloc(21);
	memcpy(rec, rect, 21);
	unsigned char *res = malloc(41);
	memcpy(res, rest, 41);

	puts("starting tests");
	printrec(rec);
	assert(putkey(rec) == 0);
	puts("putkey(rec) - PASSED");
	printf("map: %p\n", addrs.next);

	unsigned char *rec1 = fetchkey("abc");
	assert(rec1 != NULL);
	puts("fetchkey(abc) != NULL - PASSED");

	for (size_t i = 0; i < datalen(rec) + 1; i++) {
		assert(rec[i] == rec1[i]);
	}
	puts("rec1 == rec - PASSED");

	puts("adding link");
	unsigned char *rec2 = addlink(rec, "to", "def");
	assert(rec2 != NULL);
	printrec(rec2);
	for (size_t i = 0; i < datalen(res) + 1; i++) {
		assert(res[i] == rec2[i]);
	}

	puts("addlink(rec, to, def) - PASSED");
}
