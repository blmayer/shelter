#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "map.h"
#include "defs.h"
#include "record.h"

extern map addrs;
extern char debug;

unsigned char *fetchkey(unsigned char *key) {
	return get(&addrs, key);
}

int putkey(unsigned char *rec) {
	unsigned char *key_rec = data(rec);
	unsigned char *key = malloc(datalen(key_rec) + 1);
	unsigned char *key_data = data(key_rec);
	memcpy(key, key_data, datalen(key_rec));
	key[datalen(key_rec)] = 0;

	int ret = add(&addrs, key, rec);
	free(key);
	return ret;
}

int linkobjs(char* from, char *field, char *to) {
	unsigned char *from_obj = fetchkey(from);
	if (!from_obj) {
		return -1;
	}

	unsigned char *to_obj = fetchkey(to);
	if (!to_obj) {
		return -2;
	}

	unsigned char *new = addlink(from_obj, field, to_obj);
	if (!new) {
		return -3;
	}
	free(from_obj);
	return putkey(new);
}

int dump(unsigned char *rec) {
	char *path = malloc(strlen(DATA_DIR) + keylen(rec) + 1);
	strcpy(path, DATA_DIR);
	strcat(path, rec);

	printf("writing file %s\n", path);
	int file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	free(path);
	if (file < 0) {
		return -1;
	}

	write(file, rec, reclen(rec));
	return !close(file);
}

