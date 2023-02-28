#include "defs.h"
#include "methods.h"
#include "record.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int handle_request(int cli_conn) {
	unsigned char data[MAX_DATA_SIZE] = {};
	size_t n = recv(cli_conn, &data, MAX_DATA_SIZE, 0);
	if (n < 1) {
		puts("no data read");
		return -1;
	}
	printf("read %lu bytes\n", n);

	char op = getop(data);
	size_t len = reclen(data[1]);
	printf("operation: %u\n", op);
	printf("len: %li\ndata: ", len);

	for (size_t i = 0; i < n; i++) {
		printf("%u ", data[i]);
	}
	puts("");

	unsigned char *ret;
	switch (op) {
	case op_get:
		ret = fetchkey(&data+1);
		break;
	case op_put:
		ret = putkey(&data+1);
	}
	printf("operation returned %p\n", ret);

	if (!ret) {
		write(cli_conn, 0, 1);
		return 0;
	}

	return write(cli_conn, ret, reclen(ret));
}
