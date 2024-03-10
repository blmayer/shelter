#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "map.h"
#include "defs.h"
#include "record.h"
#include "alloc.h"

extern map addrs;
extern char debug;
extern unsigned char mem[100*1024*1024];

unsigned char *fetchkey(char *key) {
	int pos = get(&addrs, key);
	return &mem[pos];
}

int putkey(unsigned char *rec) {
	unsigned char *key_rec = data(rec);
	unsigned char *key = malloc(datalen(key_rec) + 1);
	unsigned char *key_data = data(key_rec);
	memcpy(key, key_data, datalen(key_rec));
	key[datalen(key_rec)] = 0;

	int pos = findpos(datalen(rec));
	int ret = add(&addrs, key, pos);
	memcpy(mem, rec, datalen(rec));

	free(key);
	return ret;
}

int linkobjs(char* from, char *field, char *to) {
	unsigned char *from_obj = fetchkey(from);
	if (!from_obj) {
		return -1;
	}

	unsigned char *new = addlink(from_obj, field, to);
	if (!new) {
		return -3;
	}
	if (new != from_obj) {
		return putkey(new);

	}
	return 0;
}

/* dumpindex expects the full record, i.e. {type_record len data...} */
int dumpindex(unsigned char *rec) {
	char path[256];
	strcpy(path, IDX_FILE);
	strncat(path, (char *)data(data(rec)), datalen(data(rec)));

	printf("writing file %s\n", path);
	printrec(rec);
	printf("reclen: %zu\n", reclen(rec));
	int file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (file < 0) {
		return -1;
	}

	write(file, rec, reclen(rec));
	return !close(file);
}

int loadindex() {
	DIR *dp;
	dp = opendir(IDX_FILE);
	if (dp == NULL) {
	  return -1;
	}

	struct dirent *entry;
	while((entry = readdir(dp))) {
		unsigned char *rec = malloc(1);
		int res = load(entry->d_name, rec);
		if (res) {
			printf("error reading %s\n", entry->d_name);
		}
		res = putkey(rec);
		if (res) {
			printf("putrec error %d\n", res);
		}
	}

	closedir(dp);
	return 0;
}
