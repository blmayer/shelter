#include "methods.h"
#include "defs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

int send_key(int conn, unsigned char *rec, unsigned int len) {
	struct stat st;

	char *path = malloc(strlen(DATA_DIR) + len + 1);
	strcpy(path, DATA_DIR);
	memcpy(path + strlen(DATA_DIR), rec, len);
	path[strlen(DATA_DIR) + len] = 0;

	/* Open the file for reading */
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
