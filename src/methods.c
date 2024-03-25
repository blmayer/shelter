#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "map.h"
#include "defs.h"
#include "record.h"
#include "alloc.h"
#include "methods.h"

extern map addrs;
extern char debug;
extern unsigned char mem[100*1024*1024];

unsigned char *fetchkey(char *key) {
	int pos = get(&addrs, key);
	DEBUGF("fetch returned pos %d\n", pos);
	if (pos < 0) return NULL;
	return &mem[pos];
}

int putkey(unsigned char *rec) {
	size_t len = reclen(rec);

	unsigned char *key_rec = data(rec);
	char key[MAX_KEY_SIZE] = {0};
	unsigned char *key_data = data(key_rec);
	memcpy(key, key_data, datalen(key_rec));

	int pos = getpos(len);
	DEBUGF("got position %d for size %lu\n", pos, len);
	int ret = add(&addrs, key, pos);
	memcpy(&mem[pos], rec, len);

	writeindex(key, pos);
	dump(pos, len);
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

/* writeindex expects the full record, i.e. {type_record len data...} */
int writeindex(char key[MAX_KEY_SIZE], int pos) {
	FILE *file = fopen(IDX_FILE, "a");
	if (!file) {
		return -1;
	}

	fprintf(file, "%d %s\n", pos, key);

	return !fclose(file);
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
