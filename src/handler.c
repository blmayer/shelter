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

	char op = data[0] >> 4;
	char len_size = data[0] & 15;
	unsigned int len = 0;
	for (int i = 0; i < len_size; i++) {
		len += data[i + 1] << (8 * i);
	}
	printf("operation: %u\n", op);
	printf("len size: %u\n", len_size);
	printf("len: %u\n", len);

	for (size_t i = 0; i < n; i++) {
		printf("%u ", data[i]);
	}
	puts("");

	int ret;
	switch (op) {
	case get:
		ret = send_key(cli_conn, &data[1 + len_size], len);
	}
	printf("operation returned %d\n", ret);

	return 0;
}
