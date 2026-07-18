/*
 * Shelter client library — builds correctly encoded messages, sends
 * them over TCP, and parses responses.
 */

#include "client.h"
#include "defs.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* ── helpers ── */

/* Write a type_key field into buf. Returns bytes written. */
static int write_key_field(unsigned char *buf, const char *str) {
	int len = strlen(str);
	buf[0] = type_key;
	*(int *)(buf + 1) = len;
	memcpy(buf + 5, str, len);
	return 5 + len;
}

/* Write a type_string field into buf. Returns bytes written. */
static int write_string_field(unsigned char *buf, const char *str) {
	int len = strlen(str);
	buf[0] = type_string;
	*(int *)(buf + 1) = len;
	memcpy(buf + 5, str, len);
	return 5 + len;
}

/* Send a message and receive a single-byte bool response ('t'/'f'). */
static int send_and_check(int fd, const unsigned char *msg, size_t len) {
	if (send(fd, msg, len, 0) < 0) return 0;
	unsigned char resp = 0;
	if (recv(fd, &resp, 1, 0) < 1) return 0;
	return resp == type_true;
}


/* ── public API ── */

int shelter_connect(const char *host, int port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return -1;

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
		close(fd);
		return -1;
	}
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

void shelter_close(int fd) {
	close(fd);
}

int shelter_build_record(const char *key, const char *field,
                         const char *value, unsigned char *buf, size_t bufsz) {
	int klen = strlen(key);
	int vlen = strlen(value);
	(void)field; /* field name not stored separately in this simple builder */

	/* record: type(1) + len(4) + key_field(5+klen) + string_field(5+vlen) */
	int total = 1 + 4 + 5 + klen + 5 + vlen;
	if ((size_t)total > bufsz) return -1;

	unsigned char *p = buf;
	*p++ = type_record;
	*(int *)p = total - 1; /* data_len excludes the type byte */
	p += 4;
	p += write_key_field(p, key);
	p += write_string_field(p, value);

	return total;
}

int shelter_put(int fd, const char *key, const char *field, const char *value) {
	unsigned char msg[MAX_DATA_SIZE];
	msg[0] = op_put;

	int reclen = shelter_build_record(key, field, value, msg + 1,
	                                  sizeof(msg) - 1);
	if (reclen < 0) return 0;

	return send_and_check(fd, msg, 1 + reclen);
}

int shelter_put_raw(int fd, const unsigned char *rec, size_t reclen) {
	unsigned char msg[MAX_DATA_SIZE];
	if (1 + reclen > sizeof(msg)) return 0;
	msg[0] = op_put;
	memcpy(msg + 1, rec, reclen);
	return send_and_check(fd, msg, 1 + reclen);
}

int shelter_get(int fd, const char *key, unsigned char *buf, size_t bufsz) {
	unsigned char msg[MAX_DATA_SIZE];
	msg[0] = op_get;
	int n = 1 + write_key_field(msg + 1, key);

	if (send(fd, msg, n, 0) < 0) return -1;

	int r = recv(fd, buf, bufsz, 0);
	if (r <= 0) return -1;
	if (r == 1 && buf[0] == '\0') return -1; /* not found */
	return r;
}

int shelter_update(int fd, const char *key, const char *field, const char *value) {
	unsigned char msg[MAX_DATA_SIZE];
	msg[0] = op_update;

	int reclen = shelter_build_record(key, field, value, msg + 1,
	                                  sizeof(msg) - 1);
	if (reclen < 0) return 0;

	return send_and_check(fd, msg, 1 + reclen);
}

int shelter_del(int fd, const char *key) {
	unsigned char msg[MAX_DATA_SIZE];
	msg[0] = op_del;
	int n = 1 + write_key_field(msg + 1, key);
	return send_and_check(fd, msg, n);
}

int shelter_link(int fd, const char *from, const char *field, const char *to) {
	unsigned char msg[MAX_DATA_SIZE];
	unsigned char *p = msg;
	*p++ = op_link;
	p += write_key_field(p, from);
	p += write_key_field(p, field);
	p += write_key_field(p, to);
	return send_and_check(fd, msg, p - msg);
}

int shelter_unlink(int fd, const char *from, const char *field, const char *to) {
	unsigned char msg[MAX_DATA_SIZE];
	unsigned char *p = msg;
	*p++ = op_unlink;
	p += write_key_field(p, from);
	p += write_key_field(p, field);
	p += write_key_field(p, to);
	return send_and_check(fd, msg, p - msg);
}

int shelter_query(int fd, const char *start_key, const char *field,
                  const char *value, unsigned char *buf, size_t bufsz) {
	unsigned char msg[MAX_DATA_SIZE];
	unsigned char *p = msg;
	*p++ = op_query;
	p += write_key_field(p, start_key);
	p += write_key_field(p, field);
	p += write_string_field(p, value);

	if (send(fd, msg, p - msg, 0) < 0) return -1;

	int r = recv(fd, buf, bufsz, 0);
	if (r <= 0) return -1;
	if (r == 1 && buf[0] == '\0') return -1;
	return r;
}