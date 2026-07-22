#include "map.h"
#include "defs.h"
#include <string.h>

extern struct freenode *freelist;

int init(map *root) {
	if (!root) {
		return 0;
	}

	root->letter = 0;
	root->pos = -1;
	root->next = NULL;

	return 1;
}

int add(map *root, char *key, int pos) {
	if (!*key) {
		root->pos = pos;
		return 0;
	}

	if (!root->next) {
		root->next = malloc(2 * sizeof(map *));
		root->next[0] = malloc(sizeof(map));
		root->next[0]->letter = *key;
		root->next[0]->pos = -1;
		root->next[0]->next = NULL;
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
	root->next = realloc(root->next, (n + 2) * sizeof(map *));
	root->next[n] = malloc(sizeof(map));
	root->next[n]->letter = *key;
	root->next[n]->pos = -1;
	root->next[n]->next = NULL;
	root->next[n + 1] = NULL;
	return add(root->next[n], ++key, pos);
}

int get(map *root, char *key) {
	if (!*key) {
		return root->pos;
	}

	if (!root->next) return -1;

	for (int i = 0; root->next[i]; i++) {
		if (root->next[i]->letter == *key) {
			return get(root->next[i], ++key);
		}
	}

	return -1;
}

int del(map *root, unsigned char *key) {
	map *curr = root;
	char found = 0;

	while (*key) {
		if (!curr->next) return 0;
		found = 0;
		for (int i = 0; curr->next[i]; i++) {
			if (curr->next[i]->letter == *key) {
				curr = curr->next[i];
				found = 1;
				break;
			}
		}
		if (!found) return 0;
		key++;
	}

	curr->pos = -1;
	return 1;
}

int destroy(map *root) {
	int i, count = 1;

	if (root->next) {
		for (i = 0; root->next[i]; i++) {
			count += destroy(root->next[i]);
		}
		free(root->next);
		root->next = NULL;
	}

	return count;
}

static void dump_walk(map *node, char *buf, int len, FILE *fp) {
	if (node->pos >= 0) {
		buf[len] = '\0';
		fprintf(fp, "%d %s\n", node->pos, buf);
	}

	if (!node->next) {
		return;
	}

	for (int i = 0; node->next[i]; i++) {
		if (len + 1 >= MAX_KEY_SIZE) {
			continue;
		}
		buf[len] = node->next[i]->letter;
		dump_walk(node->next[i], buf, len + 1, fp);
	}
}

int dumpmap(map *root, FILE *fp) {
	char buf[MAX_KEY_SIZE];
	if (!root || !fp) {
		return -1;
	}
	dump_walk(root, buf, 0, fp);
	return 0;
}
