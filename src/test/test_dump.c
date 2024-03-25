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
	unsigned char rec[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};
	unsigned char res[21] = {};

	assert(dump(rec));
	puts("dump(rec) - PASSED");

	int file = open("data/abc", O_RDONLY);
	if (file < 0) {
		return -1;
	}
	read(file, res, 21);
	close(file);

	for (size_t i = 0; i < reclen(rec); i++) {
		assert(rec[i] == res[i]);
	}
	puts("res == rec - PASSED");
}

