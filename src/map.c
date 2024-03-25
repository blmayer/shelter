#include "map.h"
#include "defs.h"
#include <string.h>

extern struct freenode *freelist;

int init(map *root) {
	if (!root) {
		return 0;
	}

	root->next = NULL;

	return 1;
}

int add(map *root, char *key, int pos) {
	if (!*key) {
		root->pos = pos;
		return 0;
	}

	if (!root->next) {
		root->next = malloc(2 * sizeof(map **));
		root->next[0] = malloc(sizeof(map *));
		root->next[0]->letter = *key;
		root->next[0]->pos = -1;
		root->next[1] = NULL;
		return add(root->next[0], ++key, pos);
	}

	int n;
	for (n = 0; root->next[n]; n++) {
		if (root->next[n]->letter == *key) {
			return add(root->next[n], ++key, pos);
		}
	}

	/* letter not found */
	root->next = realloc(root->next, n + 2 * sizeof(map **));
	root->next[n] = malloc(sizeof(map *));
	root->next[n]->letter = *key;
	root->next[n]->pos = -1;
	root->next[n + 1] = NULL;
	return add(root->next[n], ++key, pos);
}

int get(map *root, char *key) {
	if (!*key) {
		return root->pos;
	}

	for (int i = 0; root->next[i]; i++) {
		if (root->next[i]->letter == *key) {
			return get(root->next[i], ++key);
		}
	}

	return -1;
}

/* TODO: remove from array */
int del(map *root, unsigned char *key) {
	map *curr = root;
	int i = 0;
	char found;

	while (*key) {
		found = 0;
		for (i = 0; curr->next[i]; i++) {
			if (curr->next[i]->letter == *key) {
				curr = curr->next[i];
				found = 1;
				break;
			}
		}

		key++;
	}

	/* Delete only the content */
	if (found) {
		curr->pos = -1;
	}

	return 1;
}

int destroy(map *root) {
	int i, count = 1;

	/* Descend into each next element and destroy it */
	for (i = 0; root->next[i]; i++) {
		count += destroy(root->next[i]);
	}

	if (root->next) {
		free(root->next);
	}

	return count;
}
