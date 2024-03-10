#include "defs.h"
#include "handler.h"
#include "map.h"
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

char debug = 0;
int server;

map addrs;
unsigned char mem[100*1024*1024] = {};	/* 100MiB */
struct freenode *freelist;
int dbfile;

void sig_handler() {
	puts("closing server");
	close(server);
	close(dbfile);
	exit(1);
}

int main(void) {
	if (getenv("DEBUG")) {
		debug = 1;
	}

	if (!init(&addrs)) {
		puts("map init error");
		return -1;
	}

	freelist = malloc(sizeof(struct freenode));
 	freelist->pos = 0;
	freelist->size = 100*1024*1024;

	int f = stat(DB_FILE, NULL);
	if (f < 0) {
		dbfile = open(DB_FILE, O_RDWR | O_CREAT, 0777);
	} else {
		dbfile = open(DB_FILE, O_RDWR);
		read(dbfile, &mem, 100*1024*1024);
	}
	if (dbfile < 0) {
		return -1;
	}

	int portnum = 8080;

	char *port = getenv("PORT");
	if (port != NULL) {
		portnum = atoi(port);
	}

	signal(SIGINT, sig_handler);
	signal(SIGKILL, sig_handler);
	signal(SIGSTOP, sig_handler);

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server < 0) {
		perror("socket creation failed");
		return 0;
	}

	/* mAke server reuse addresses */
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	struct sockaddr_in serv, client;

	serv.sin_family = AF_INET;
	serv.sin_port = htons(portnum);
	serv.sin_addr.s_addr = INADDR_ANY;

	if (bind(server, (struct sockaddr *)&serv, 16) < 0) {
		perror("bind failed");
		close(server);
		return 0;
	}

	/* DEBUG("db is listening on port %d\n", portnum); */

	listen(server, 10);
	socklen_t cli_len;
	int conn;

	while (1) {
		/* Get the new connection */
		cli_len = sizeof(client);
		conn = accept(server, (struct sockaddr *)&client, &cli_len);

		/*  We got a connection! Check if it is OK */
		if (conn < 0) {
			perror("couldn't connect");
			continue;
		}

		/* Fork the connection */
		pid_t conn_pid = fork();

		/* Manipulate those processes */
		if (conn_pid < 0) {
			perror("couldn't fork");
		}

		if (conn_pid == 0) {
			/* This is the child process */
			/* Close the parent connection */
			close(server);

			/* Process the response */
			handle_request(conn);

			/* Close the client socket, not needed anymore */
			close(conn);
			exit(0);
		} else {
			/* This is the master process */
			/* Close the connection */
			close(conn);
		}
	}

	return 0;
}
