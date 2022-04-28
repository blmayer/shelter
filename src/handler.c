#include "defs.h"
#include "methods.h"
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
	size_t len = getlen(data);
	printf("operation: %u\n", op);
	printf("len: %li\ndata: ", len);

	for (size_t i = 0; i < n; i++) {
		printf("%u ", data[i]);
	}
	puts("");

	int ret;
	switch (op) {
	case get:
		ret = send_key(cli_conn, data);
		break;
	case put:
		ret = put_key(cli_conn, data);
	}
	printf("operation returned %d\n", ret);

	return 0;
}
