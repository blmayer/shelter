#include <string.h>
#include "map.h"

int init(map *root) {
	if (!root) {
		return 0;
	}

	root->next = NULL;
	root->content = NULL;

	return 1;
}

int add(map *root, unsigned char *key, unsigned char *data) {
	if (!*key) {
		root->content = data;
		return 0;
	}

	if (!root->next) {
		root->next = malloc(2*sizeof(map**));
		root->next[0] = malloc(sizeof(map*));
		root->next[0]->letter = *key;
		root->next[1] = NULL;
		return add(root->next[0], ++key, data);
	}

	int n;
	for (n = 0; root->next[n]; n++) {
		if (root->next[n]->letter == *key) {
			return add(root->next[n], ++key, data);
		}
	}

	/* letter not found */
	root->next = realloc(root->next, n+2 * sizeof(map**));
	root->next[n] = malloc(sizeof(map*));
	root->next[n]->letter = *key;
	root->next[n+1] = NULL;
	return add(root->next[n], ++key, data);
}

unsigned char *get(map *root, char *key) {
	if (!*key) {
		return root->content;
	}

	for (int i = 0; root->next[i]; i++) {
		if (root->next[i]->letter == *key) {
			return get(root->next[i], ++key);
		}
	}

	return NULL;
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
	if (found && curr->content) {
		free(curr->content);
		curr->content = NULL;
	}

	return 1;
}

int destroy(map *root) {
	int i, count = 1;

	/* Descend into each next element and destroy it */
	for (i = 0; root->next[i]; i++) {
		count += destroy(root->next[i]);
	}
	
	/* Erase itself	*/
	if (root->content) {
		free(root->content);
	}
	if (root->next) {
		free(root->next);
	}
	
	return count;
}
