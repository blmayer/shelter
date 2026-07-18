#include "defs.h"
#include "methods.h"
#include "record.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

static int reply_bool(int fd, int ok) {
	unsigned char b = ok ? type_true : type_false;
	return write(fd, &b, 1);
}

static int reply_bytes(int fd, const unsigned char *data, size_t len) {
	if (!data || len == 0) {
		unsigned char zero = '\0';
		return write(fd, &zero, 1);
	}
	return write(fd, data, len);
}

int handle_request(int cli_conn) {
	unsigned char msg[MAX_DATA_SIZE] = {};
	size_t n = recv(cli_conn, &msg, MAX_DATA_SIZE, 0);
	if (n < 1) {
		puts("no data read");
		return -1;
	}
	printf("read %lu bytes, op '%c'\n", (unsigned long)n, msg[0]);

	char op = msg[0];
	unsigned char *payload = &msg[1];
	unsigned char *ret;
	int qlen;

	switch (op) {
	case op_get:
		ret = get_from_payload(payload);
		return reply_bytes(cli_conn, ret, ret ? reclen(ret) : 0);

	case op_put:
		return reply_bool(cli_conn, putkey(payload) == 0);

	case op_update:
		return reply_bool(cli_conn, updatekey(payload) == 0);

	case op_del:
		return reply_bool(cli_conn, del_from_payload(payload) == 0);

	case op_link:
		return reply_bool(cli_conn, link_from_payload(payload) == 0);

	case op_unlink:
		return reply_bool(cli_conn, unlink_from_payload(payload) == 0);

	case op_query:
		qlen = (int)(n - 1);
		ret = query(payload, &qlen);
		if (ret) {
			int w = reply_bytes(cli_conn, ret, (size_t)qlen);
			free(ret);
			return w;
		}
		return reply_bytes(cli_conn, NULL, 0);

	default:
		printf("unknown operation: %c (%u)\n", op, (unsigned)op);
		return reply_bytes(cli_conn, NULL, 0);
	}
}
