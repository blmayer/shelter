#include "defs.h"
#include <dirent.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern int dbfile;
extern unsigned char mem[100*1024*1024];

char getop(unsigned char *rec) {
	return rec[0];
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
		data++;
		return *(int *)data;
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

int parse_key_field(unsigned char **p, char *buf, size_t bufsz) {
	if (gettype(*p) != type_key) return -1;
	int klen = datalen(*p);
	if (klen <= 0 || (size_t)klen >= bufsz) return -1;
	memcpy(buf, data(*p), klen);
	buf[klen] = '\0';
	*p += 1 + sizeof(int) + klen;
	return 0;
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

/*
 * Append a named link. Target is the key string (not arena pos/handle).
 * See DESIGN.md "Link target identity" — do not change without updating that.
 */
unsigned char *addlink(unsigned char *fromrec, char *field, char *to) {
	int from_len = datalen(fromrec) + 1;
	/* link overhead: type_link(1) + linklen(4) + type_key(1) + fieldlen(4) + field + type_key(1) + tolen(4) + to */
	int link_data_len = 2 + 2*sizeof(int) + strlen(field) + strlen(to);
	int new_len = from_len + 1 + sizeof(int) + link_data_len;

	fromrec = realloc(fromrec, new_len);
	unsigned char *old = fromrec;

	/* update record length */
	*(int *)(fromrec + 1) = new_len - 1;

	/* append link at end of existing data */
	unsigned char *p = fromrec + from_len;
	*p++ = type_link;
	*(int *)p = link_data_len;
	p += sizeof(int);
	*p++ = type_key;
	*(int *)p = strlen(field);
	p += sizeof(int);
	memcpy(p, field, strlen(field));
	p += strlen(field);
	*p++ = type_key;
	*(int *)p = strlen(to);
	p += sizeof(int);
	memcpy(p, to, strlen(to));

	return old;
}

/* Remove a link from a record that matches field and target.
 * Returns the (possibly realloc'd) record, or NULL if not found. */
unsigned char *rmlink(unsigned char *fromrec, char *field, char *to) {
	int rec_total = datalen(fromrec) + 1;
	unsigned char *p = fromrec + 5; /* skip type_record + 4-byte len */
	int remaining = rec_total - 5;

	while (remaining > 0) {
		enum type t = gettype(p);
		int field_total;

		if (t == type_false || t == type_true) {
			field_total = 1;
		} else {
			int dlen = datalen(p);
			field_total = 1 + sizeof(int) + dlen;
		}

		if (t == type_link) {
			/* parse link: type_key + len + field_name + type_key + len + target */
			unsigned char *ldata = p + 1 + sizeof(int);
			int flen = *(int *)(ldata + 1);
			char *fname = (char *)(ldata + 1 + sizeof(int));
			unsigned char *tpart = ldata + 1 + sizeof(int) + flen;
			int tlen = *(int *)(tpart + 1);
			char *tname = (char *)(tpart + 1 + sizeof(int));

			if ((int)strlen(field) == flen && memcmp(fname, field, flen) == 0 &&
			    (int)strlen(to) == tlen && memcmp(tname, to, tlen) == 0) {
				/* found — shift remaining data left over this link */
				int after = remaining - field_total;
				if (after > 0) {
					memmove(p, p + field_total, after);
				}
				int new_total = rec_total - field_total;
				*(int *)(fromrec + 1) = new_total - 1;
				fromrec = realloc(fromrec, new_total);
				return fromrec;
			}
		}

		p += field_total;
		remaining -= field_total;
	}

	return NULL; /* link not found */
}

unsigned char *next(unsigned char *rec) {
	return rec + datalen(rec);
}

/* dump expects the full record, i.e. {type_record len data...} */
int dump(int pos, size_t size) {
	lseek(dbfile, pos, SEEK_SET);
	return write(dbfile, &mem[pos], size);
}

int load(char *key, unsigned char **rec) {
	char name[256] = {0};
	strncpy(name, key, sizeof(name) - 1);
	FILE *file = fopen(name, "rb");
	if (!file) {
		return -1;
	}

	fseek(file, 0, SEEK_END);
	size_t len = ftell(file);
	rewind(file);

	*rec = realloc(*rec, len);
	if (!*rec) {
		fclose(file);
		return -1;
	}
	fread(*rec, 1, len, file);
	fclose(file);

	return 0;
}
