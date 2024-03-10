#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

extern struct freenode *freelist;
extern char debug;

int getpos(int size) {
	struct freenode *ptr = freelist;
	if (ptr->size >= size) {
		DEBUG("space available\n");
		ptr->size -= size;
		ptr->pos += size;
		if (ptr->size == 0) {
			freelist = ptr->next;
			free(ptr);
		}
		return ptr->pos - size;
	}

	struct freenode *prev;
	while (ptr->next) {
		prev = ptr;
		ptr = ptr->next;
		if (ptr->size >= size) {
			ptr->size -= size;
			ptr->pos += size;
			if (ptr->size == 0) {
				prev->next = ptr->next;
				free(ptr);
			}
			return ptr->pos - size;
		}
	}
	return -1;
}

/*
 * Case A: new free is at end of a free zone.
 * +--------+--------+-----------+------------+
 * |  free  |  new   |           |    free    |
 * +--------+--------+-----------+------------+
 *
 * Case B: new free is at start of a free zone.
 * +--------+-------------+------+------------+
 * |  free  |             | new  |    free    |
 * +--------+-------------+------+------------+
 *
 * Case C: new free is at start.
 * +-------+---------------------+------------+
 * |  new  |                     |    free    |
 * +-------+---------------------+------------+
 *
 * Case D: nota.
 * +--------+-------+------+-----+------------+
 * |  free  |       | new  |     |    free    |
 * +--------+-------+------+-----+------------+
 */
int freepos(int pos, int size) {
	struct freenode *ptr = freelist;

	/* case D */
	if (pos == 0) {
		if (ptr->pos > pos + size) {
			DEBUG("case D1\n");
			struct freenode *next = malloc(sizeof(struct freenode));
			next->pos = ptr->pos;
			next->size = ptr->size;
			next->next = ptr;
			freelist->next = next;
			freelist->pos = 0;
			freelist->size = size;
		} else if (ptr->pos == size) {
			DEBUG("case D2\n");
			ptr->pos = 0;
			ptr->size += size;
		}
		return 0;
	}
free:
	/* case A */
	if (ptr->pos + ptr->size == pos) {
		DEBUG("case A\n");
		ptr->size += size;
		return pos;
	}

	/* case B */
	if (ptr->pos == pos + size) {
		DEBUG("case B\n");
		ptr->next->pos -= size;
		ptr->next->size += size;
		return pos + size;
	}

	if (ptr->next) {
		ptr = ptr->next;
		goto free;
	}

	/* case D */
	DEBUG("case D\n");
	ptr->next = malloc(sizeof(struct freenode));
	ptr->next->pos = pos;
	ptr->next->size = size;
	ptr->next->next = NULL;

	return pos;
}
