#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
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

	/* copy to heap since addlink uses realloc and from_obj points into mem[] */
	size_t len = reclen(from_obj);
	unsigned char *copy = malloc(len);
	if (!copy) return -2;
	memcpy(copy, from_obj, len);

	/* free old space */
	int oldpos = get(&addrs, from);
	freepos(oldpos, len);

	unsigned char *new = addlink(copy, field, to);
	if (!new) {
		free(copy);
		return -3;
	}

	/* store the enlarged record back */
	size_t newlen = reclen(new);
	int pos = getpos(newlen);
	if (pos < 0) {
		free(new);
		return -4;
	}
	memcpy(&mem[pos], new, newlen);
	add(&addrs, from, pos);
	dump(pos, newlen);
	free(new);

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
	FILE *fp = fopen(IDX_FILE, "r");
	if (!fp) {
		return -1;
	}

	int pos;
	char key[MAX_KEY_SIZE];
	while (fscanf(fp, "%d %s\n", &pos, key) == 2) {
		add(&addrs, key, pos);
	}

	fclose(fp);
	return 0;
}

int updatekey(unsigned char *rec) {
	unsigned char *key_rec = data(rec);
	char key[MAX_KEY_SIZE] = {0};
	unsigned char *key_data = data(key_rec);
	memcpy(key, key_data, datalen(key_rec));

	int oldpos = get(&addrs, key);
	if (oldpos < 0) {
		return -1; /* key does not exist */
	}

	/* free old space */
	size_t oldlen = reclen(&mem[oldpos]);
	freepos(oldpos, oldlen);

	/* allocate new space */
	size_t newlen = reclen(rec);
	int pos = getpos(newlen);
	if (pos < 0) return -2;

	memcpy(&mem[pos], rec, newlen);
	add(&addrs, key, pos);

	writeindex(key, pos);
	dump(pos, newlen);
	return 0;
}

int delkey(char *key) {
	int pos = get(&addrs, key);
	if (pos < 0) {
		return -1; /* not found */
	}

	size_t len = reclen(&mem[pos]);
	freepos(pos, len);
	del(&addrs, (unsigned char *)key);

	return 0;
}

unsigned char *get_from_payload(unsigned char *payload) {
	char key[MAX_KEY_SIZE] = {0};
	unsigned char *p = payload;
	if (parse_key_field(&p, key, sizeof(key)) < 0) return NULL;
	return fetchkey(key);
}

int del_from_payload(unsigned char *payload) {
	char key[MAX_KEY_SIZE] = {0};
	unsigned char *p = payload;
	if (parse_key_field(&p, key, sizeof(key)) < 0) return -1;
	return delkey(key);
}

int link_from_payload(unsigned char *payload) {
	char from[MAX_KEY_SIZE] = {0};
	char field[MAX_KEY_SIZE] = {0};
	char to[MAX_KEY_SIZE] = {0};
	unsigned char *p = payload;
	if (parse_key_field(&p, from, sizeof(from)) < 0) return -1;
	if (parse_key_field(&p, field, sizeof(field)) < 0) return -1;
	if (parse_key_field(&p, to, sizeof(to)) < 0) return -1;
	return linkobjs(from, field, to);
}

int unlink_from_payload(unsigned char *payload) {
	char from[MAX_KEY_SIZE] = {0};
	char field[MAX_KEY_SIZE] = {0};
	char to[MAX_KEY_SIZE] = {0};
	unsigned char *p = payload;
	if (parse_key_field(&p, from, sizeof(from)) < 0) return -1;
	if (parse_key_field(&p, field, sizeof(field)) < 0) return -1;
	if (parse_key_field(&p, to, sizeof(to)) < 0) return -1;
	return unlinkobjs(from, field, to);
}

int unlinkobjs(char *from, char *field, char *to) {
	unsigned char *from_obj = fetchkey(from);
	if (!from_obj) {
		return -1;
	}

	size_t len = reclen(from_obj);
	unsigned char *copy = malloc(len);
	if (!copy) return -2;
	memcpy(copy, from_obj, len);

	unsigned char *new = rmlink(copy, field, to);
	if (!new) {
		free(copy);
		return -3; /* link not found */
	}

	/* free old space */
	int oldpos = get(&addrs, from);
	freepos(oldpos, len);

	/* store the shrunk record back */
	size_t newlen = reclen(new);
	int pos = getpos(newlen);
	if (pos < 0) {
		free(new);
		return -4;
	}
	memcpy(&mem[pos], new, newlen);
	add(&addrs, from, pos);
	dump(pos, newlen);
	free(new);

	return 0;
}

/*
 * Query: graph traversal starting at a key.
 *
 * Wire format (after op byte):
 *   {type_key, len, start_key, steps...}
 * Each step: {type_key, len, field_name, type_string, len, value_pattern}
 *   value_pattern "*" means wildcard (match any target).
 *
 * Returns a heap-allocated buffer:
 *   {type_list, count(4), record1, record2, ...}
 * Caller must free the buffer. *outlen receives total byte length.
 */
unsigned char *query(unsigned char *msg, int *outlen) {
	/* parse start key */
	unsigned char *p = msg;
	if (gettype(p) != type_key) return NULL;
	int sklen = datalen(p);
	char start[MAX_KEY_SIZE] = {0};
	memcpy(start, data(p), sklen);
	p += 1 + sizeof(int) + sklen;

	/* collect current set of keys */
	int capacity = 64;
	int count = 0;
	char **keys = malloc(capacity * sizeof(char *));
	keys[0] = strdup(start);
	count = 1;

	/* process each step */
	while ((p - msg) < *outlen) {
		if (gettype(p) != type_key) break;
		int flen = datalen(p);
		char field[MAX_KEY_SIZE] = {0};
		memcpy(field, data(p), flen);
		p += 1 + sizeof(int) + flen;

		/* read value pattern */
		if (gettype(p) != type_string) break;
		int vlen = datalen(p);
		char val[MAX_KEY_SIZE] = {0};
		memcpy(val, data(p), vlen);
		p += 1 + sizeof(int) + vlen;
		int wildcard = (vlen == 1 && val[0] == '*');

		/* for each key in current set, find matching links */
		int new_cap = 64;
		int new_count = 0;
		char **new_keys = malloc(new_cap * sizeof(char *));

		for (int i = 0; i < count; i++) {
			unsigned char *rec = fetchkey(keys[i]);
			if (!rec) continue;

			/* walk fields of the record */
			int rec_total = reclen(rec);
			unsigned char *f = rec + 5;
			int rem = rec_total - 5;

			while (rem > 0) {
				enum type t = gettype(f);
				int ftotal;
				if (t == type_false || t == type_true) {
					ftotal = 1;
				} else {
					ftotal = 1 + sizeof(int) + datalen(f);
				}

				if (t == type_link) {
					unsigned char *ld = f + 1 + sizeof(int);
					int lflen = datalen(ld);
					char *lfname = (char *)data(ld);
					unsigned char *tp = ld + 1 + sizeof(int) + lflen;
					int ltlen = datalen(tp);
					char *ltname = (char *)data(tp);

					if (lflen == (int)strlen(field) && memcmp(lfname, field, lflen) == 0) {
						char target[MAX_KEY_SIZE] = {0};
						memcpy(target, ltname, ltlen);

						if (wildcard || strcmp(val, target) == 0) {
							if (new_count >= new_cap) {
								new_cap *= 2;
								new_keys = realloc(new_keys, new_cap * sizeof(char *));
							}
							new_keys[new_count++] = strdup(target);
						}
					}
				}

				f += ftotal;
				rem -= ftotal;
			}
			free(keys[i]);
		}
		free(keys);
		keys = new_keys;
		count = new_count;
	}

	/* build response: {type_list, count(4), records...} */
	/* first pass: compute total size */
	int total = 1 + sizeof(int); /* type_list + count */
	for (int i = 0; i < count; i++) {
		unsigned char *rec = fetchkey(keys[i]);
		if (rec) {
			total += reclen(rec);
		}
	}

	unsigned char *result = malloc(total);
	if (!result) {
		for (int i = 0; i < count; i++) free(keys[i]);
		free(keys);
		*outlen = 0;
		return NULL;
	}

	unsigned char *w = result;
	*w++ = type_list;
	*(int *)w = count;
	w += sizeof(int);

	for (int i = 0; i < count; i++) {
		unsigned char *rec = fetchkey(keys[i]);
		if (rec) {
			int rlen = reclen(rec);
			memcpy(w, rec, rlen);
			w += rlen;
		}
		free(keys[i]);
	}
	free(keys);

	*outlen = total;
	return result;
}
