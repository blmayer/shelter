#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdlib.h>

typedef struct node map;

struct node {
	char letter;
	int pos;
	map **next;
};

int init(map *root);

int add(map *root, char *key, int pos);

int get(map *root, char *key);

int del(map *root, unsigned char *key);

int destroy(map *root);

/* Write all key→pos entries to fp as "<pos> <key>\n" lines. */
int dumpmap(map *root, FILE *fp);

#endif
