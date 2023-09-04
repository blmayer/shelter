#include "defs.h"
#include <dirent.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char getop(unsigned char *rec) {
	return rec[0] >> 4;
}

enum type gettype(unsigned char *rec) {
	return rec[0];
}

int typelen(unsigned char *data) {
	switch (gettype(data)) {
	case type_false:
		return 1;
	case type_true:
		return 1;
	case type_int:
		return 1;
	case type_decimal:
		return 1;
	case type_link:
		return 5;
	default:
		return 5;
	}
}

unsigned char *data(unsigned char *rec) {
	return rec + typelen(rec);
}

int datalen(unsigned char *data) {
	enum type t = gettype(data);
	switch (t) {
	case type_false:
		return 0;
	case type_true:
		return 0;
	case type_int:
		return 4;
	case type_decimal:
		return 4;
	case type_link:
		return 8;
	case type_key:
		data++;
		return *(int *)data;
	default:
		data++;
		return *(int *)data;
	}
}

size_t reclen(unsigned char *rec) {
	return datalen(rec) + 1;
}

void printrec(unsigned char *rec) {
	size_t i;
	for (i = 0; i < reclen(rec); i++) {
		printf("%2zu ", i);
	}
	puts("");
	for (i = 0; i < reclen(rec); i++) {
		if (rec[i] > 32 && rec[i] < 128) {
			printf("%2c ", rec[i]);
		} else {
			printf("%02d ", rec[i]);
		}
	}
	puts("");
}

unsigned char *addlink(unsigned char *fromrec, char *field, char *to) {
	int from_len = datalen(fromrec) + 1;
	int new_len = from_len + 15 + strlen(field) + strlen(to);

	// copy data
	fromrec = realloc(fromrec, new_len);
	unsigned char *old = fromrec;

	// update len part
	fromrec ++;
	*fromrec = new_len - 1;
	fromrec += sizeof(int);

	// add fromrec record at end
	fromrec += from_len - sizeof(int) - 1;
	*fromrec = type_link;
	fromrec ++;
	*fromrec = strlen(field) + 1 + 2*sizeof(int) + strlen(to);
	fromrec += sizeof(int);
	*fromrec = type_key;
	fromrec ++;
	*fromrec = strlen(field);
	fromrec += sizeof(int);
	memcpy(fromrec, field, strlen(field));
	fromrec += strlen(field);
	*fromrec = type_key;
	fromrec ++;
	*fromrec = strlen(to);
	fromrec += sizeof(int);
	memcpy(fromrec, to, strlen(to));

	return old;
}

unsigned char *next(unsigned char *rec) {
	return rec + datalen(rec);
}

/* dump expects the full record, i.e. {type_record len data...} */
int dump(unsigned char *rec) {
	char path[256];
	strcpy(path, DATA_DIR);
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

int load(char *key, unsigned char *rec) {
	char name[256];
	strcpy(name, DATA_DIR);
	strcat(name, key);
	FILE *file = fopen(name, "r");
	if (!file) {
		return -1;
	}

	fseek(file, 0, SEEK_END);
	size_t reclen = ftell(file);
	rewind(file);

	rec = realloc(rec, reclen);
	fread(rec, 1, reclen, file);

	return !fclose(file);
}
