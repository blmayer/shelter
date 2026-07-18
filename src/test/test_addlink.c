#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../map.h"
#include "../defs.h"
#include "../methods.h"
#include "../record.h"

char debug = 0;
map addrs;
int dbfile;
struct freenode *freelist;
unsigned char mem[100 * 1024 * 1024] = {};

int main(void) {
	init(&addrs);
	freelist = malloc(sizeof(struct freenode));
	freelist->pos = 0;
	freelist->size = 100 * 1024 * 1024;

	/* link data: type_key(1)+len(4)+"to"(2) + type_key(1)+len(4)+"def"(3) = 15 bytes */
	unsigned char rect[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};
	unsigned char rest[] = {type_record, 40, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z', type_link, 15, 0, 0, 0, type_key, 2, 0, 0, 0, 't', 'o', type_key, 3, 0, 0, 0, 'd', 'e', 'f'};

	unsigned char *rec = malloc(21);
	memcpy(rec, rect, 21);

	puts("starting tests");
	printrec(rec);
	assert(putkey(rec) == 0);
	puts("putkey(rec) - PASSED");

	unsigned char *rec1 = fetchkey("abc");
	assert(rec1 != NULL);
	puts("fetchkey(abc) != NULL - PASSED");

	puts("adding link");
	unsigned char *rec2 = addlink(rec, "to", "def");
	assert(rec2 != NULL);
	printrec(rec2);

	size_t expected_len = sizeof(rest);
	size_t actual_len = reclen(rec2);
	printf("expected len = %zu, actual len = %zu\n", expected_len, actual_len);
	assert(actual_len == expected_len);

	printf("expected: ");
	for (size_t i = 0; i < expected_len; i++) printf("%02x ", rest[i]);
	printf("\n");
	printf("actual:   ");
	for (size_t i = 0; i < actual_len; i++) printf("%02x ", rec2[i]);
	printf("\n");
	fflush(stdout);

	for (size_t i = 0; i < expected_len; i++) {
		assert(rest[i] == rec2[i]);
	}
	puts("addlink(rec, to, def) - PASSED");

	free(rec2);
}
