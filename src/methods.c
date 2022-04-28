#include "methods.h"
#include "defs.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

int send_key(int conn, unsigned char *rec) {
	size_t len = datalen(rec);
	rec = data(rec);
	struct stat st;

	char *path = malloc(strlen(DATA_DIR) + len + 1);
	strcpy(path, DATA_DIR);
	memcpy(path + strlen(DATA_DIR), rec, len);
	path[strlen(DATA_DIR) + len] = 0;

	printf("reading file %s\n", path);
	stat(path, &st);
	int page_file = open(path, O_RDONLY);
	free(path);
	if (page_file < 0) {
		return -1;
	}

	printf("file size %li bytes\n", st.st_size);

	int sent = sendfile(conn, page_file, 0, st.st_size);
	printf("sent %d bytes\n", sent);

	return 0;
}

int put_key(int conn, unsigned char *rec) {
	rec = data(rec);
	size_t key_len = datalen(rec);

	unsigned char *key = rec;
	key = data(key);

	char *path = malloc(strlen(DATA_DIR) + key_len + 1);
	strcpy(path, DATA_DIR);
	memcpy(path + strlen(DATA_DIR), key, key_len);
	path[strlen(DATA_DIR) + key_len] = 0;

	printf("writing file %s\n", path);
	int page_file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	free(path);
	if (page_file < 0) {
		return -1;
	}

	rec = next(rec);
	size_t obj_len = getlen(rec);

	printf("writing data\n");
	for (size_t i = 0; i < obj_len; i++) {
		printf("%d ", rec[i]);
	}
	puts("");

	size_t sent = write(page_file, rec, obj_len);
	if (sent != obj_len) {
		return -2;
	}

	return 0;
}

char getop(unsigned char *rec) {
	return rec[0] >> 4;
}

size_t getlensize(unsigned char *rec) {
	return rec[0] & 15;
}

size_t datalen(unsigned char *rec) {
	size_t len_size = getlensize(rec);
	size_t len = 0;
	for (size_t i = 0; i < len_size; i++) {
		len += rec[i + 1] << (8 * i);
	}
	return len;
}

size_t getlen(unsigned char *rec) {
	return 1 + datalen(rec) + getlensize(rec);
}

unsigned char *next(unsigned char *rec) {
	return &rec[getlen(rec)];
}

unsigned char *data(unsigned char *rec) {
	return &rec[1 + getlensize(rec)];
}