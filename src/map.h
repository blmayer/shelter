#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdlib.h>

typedef struct node map;

struct node {
	int count;
	char letter;
	unsigned char *content;
	map *next;
};

int init(map *root);

int add(map *root, unsigned char *key, unsigned char *data);

unsigned char *get(map *root, unsigned char *key);

int delete(map *root, unsigned char *key);

int destroy(map *root);

#endif
