#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"

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
			return 1;
		default:
			return 5;
	}
}

unsigned char *data(unsigned char *rec) {
	return rec+typelen(rec);
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
			return *(int*)data;
		default:
			data++;
			return *(int*)data;
	}
}

size_t reclen(unsigned char *rec) {
	rec++;
	return *(size_t*)rec;
}

void printrec(unsigned char *rec) {
	int i;
	for (i = 0; i < datalen(rec) + typelen(rec); i++) {
		printf("%2d ", i);
	}
	puts("");
	for (i = 0; i < datalen(rec) + typelen(rec); i++) {
		if (rec[i] > 30 && rec[i] < 128) {
			printf("%2c ", rec[i]);
		} else {
			printf("%.2x ", rec[i]);
		}
	}
	puts("");
}

unsigned char *addlink(unsigned char *from, char *field, unsigned char *addr) {
	int from_len = datalen(from) + typelen(from);
	int new_len = from_len + typelen((unsigned char[]){type_key}) + strlen(field) + 1 + sizeof(char *);

	// update len part
	unsigned char *new = malloc(new_len);
	unsigned char *orig = from;
	unsigned char *start = new;
	*new++ = *from++;
	*new = new_len - typelen(orig);
	new += sizeof(int);
	from += sizeof(int);

	// copy data
	memcpy(new, from, datalen(orig));

	// add new record at end
	new += datalen(orig);
	*new = type_link;
	new++;
	*new = strlen(field) + 1 + sizeof(char *);
	new += sizeof(int);
	memcpy(new, field, strlen(field));
	new += strlen(field);
	*new = 0;
	new++;
	memcpy(new, &addr, sizeof(char *));
	
	return start;
}

unsigned char *next(unsigned char *rec) {
	return rec + datalen(rec);
}

