#include <stdio.h>
#include <assert.h>
#include "../map.h"
#include "../defs.h"
#include "../methods.h"
#include "../record.h"

char debug = 1;
map addrs;
int dbfile;
int idxfile;
struct freenode *freelist;
unsigned char mem[100 * 1024 * 1024] = {}; /* 100MiB */

int main(void) {
	init(&addrs);
	freelist = malloc(sizeof(struct freenode));
	freelist->pos = 0;
	freelist->size = 100 * 1024 * 1024;
	unsigned char rec[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};
	unsigned char rec1[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'd', type_string, 3, 0, 0, 0, 'z', 'y', 'x'};

	puts("starting tests");
	printrec(rec);
	assert(putkey(rec) == 0);
	puts("putkey(rec) - PASSED");
	printf("map: %p\n", addrs.next);

	unsigned char *res = fetchkey("abc");
	assert(res != NULL);
	puts("fetchkey(abc) != NULL - PASSED");
	printrec(res);

	for (size_t i = 0; i < reclen(rec); i++) {
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
	printrec(res);

	for (size_t i = 0; i < reclen(rec); i++) {
		assert(rec[i] == res[i]);
	}
	puts("res == rec - PASSED");

	res = fetchkey("abd");
	assert(res != NULL);
	puts("fetchkey(abd) != NULL - PASSED");
	printrec(res);

	for (size_t i = 0; i < reclen(rec1); i++) {
		assert(rec1[i] == res[i]);
	}
	puts("res == rec1 - PASSED");
}

