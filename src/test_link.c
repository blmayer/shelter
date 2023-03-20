#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "defs.h"
#include "handler.h"
#include "methods.h"
#include "record.h"
#include "map.h"

map addrs;

int main(void) {
	init(&addrs);
	unsigned char data[] = {type_record, 16, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', 4, 3, 0, 0, 0, 'x', 'y', 'z'};
	unsigned char data2[] = {type_record, 16, 0, 0, 0, type_key, 3, 0, 0, 0, 'd', 'e', 'f', 4, 3, 0, 0, 0, 'f', 'f', 'f'};
	
	unsigned char *rec = malloc(21);
	unsigned char *rec2 = malloc(21);
	memcpy(rec, data, 21);
	memcpy(rec2, data2, 21);

	assert(putkey(rec) == 1);
	assert(putkey(rec2) == 1);
	assert(linkobjs("abc", "to", "def") != 0);
	puts("linkkey() - PASSED");

	unsigned char *obj = fetchkey("abc");
	unsigned char **ptr = (unsigned char **)(obj+29);
	unsigned char *tgt = *ptr;
	for (int i = 0; i < 12; i++) {
		assert(rec2[i] == tgt[i]);
	}
	puts("tgt == rec2 - PASSED");

	unsigned char *obj2 = fetchkey("def");
	for (int i = 0; i < 12; i++) {
		assert(obj2[i] == tgt[i]);
	}
	puts("tgt == obj2 - PASSED");
}

