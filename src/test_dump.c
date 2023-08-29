#include "defs.h"
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
	unsigned char rec[] = {'a', 'b', 'c', 0, 4, 3, 0, 0, 0, 'x', 'y', 'z'};
	unsigned char res[12] = {};
	unsigned char *key = "abc";

	assert(dump(rec));
	puts("dump(rec) - PASSED");

	int file = open("data/abc", O_RDONLY);
	if (file < 0) {
		return -1;
	}
	read(file, res, 12);
	close(file);

	for (int i = 0; i < reclen(rec); i++) {
		assert(rec[i] == res[i]);
	}
	puts("res == rec - PASSED");
}

