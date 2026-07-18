#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

extern struct freenode *freelist;
extern char debug;

int getpos(int size) {
	struct freenode *ptr = freelist;
	if (ptr->size >= size) {
		DEBUG("space available\n");
		int result = ptr->pos;
		ptr->size -= size;
		ptr->pos += size;
		if (ptr->size == 0) {
			freelist = ptr->next;
			free(ptr);
		}
		return result;
	}

	struct freenode *prev;
	while (ptr->next) {
		prev = ptr;
		ptr = ptr->next;
		if (ptr->size >= size) {
			int result = ptr->pos;
			ptr->size -= size;
			ptr->pos += size;
			if (ptr->size == 0) {
				prev->next = ptr->next;
				free(ptr);
			}
			return result;
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

	/* case D: freed block at position 0 */
	if (pos == 0) {
		if (ptr->pos == size) {
			/* adjacent: just extend the existing free node backward */
			DEBUG("case D2\n");
			ptr->pos = 0;
			ptr->size += size;
		} else if (ptr->pos > size) {
			/* gap: insert a new node at the front */
			DEBUG("case D1\n");
			struct freenode *new = malloc(sizeof(struct freenode));
			new->pos = 0;
			new->size = size;
			new->next = freelist;
			freelist = new;
		}
		return 0;
	}
	while (ptr) {
		/* case A: freed block is right after this free node */
		if (ptr->pos + ptr->size == pos) {
			DEBUG("case A\n");
			ptr->size += size;
			/* also merge with next node if now adjacent */
			if (ptr->next && ptr->pos + ptr->size == ptr->next->pos) {
				struct freenode *old = ptr->next;
				ptr->size += old->size;
				ptr->next = old->next;
				free(old);
			}
			return pos;
		}

		/* case B: freed block is right before this free node */
		if (pos + size == ptr->pos) {
			DEBUG("case B\n");
			ptr->pos = pos;
			ptr->size += size;
			return pos;
		}

		/* insert before next if the gap is between ptr and ptr->next */
		if (!ptr->next || ptr->next->pos > pos) {
			DEBUG("case D\n");
			struct freenode *new = malloc(sizeof(struct freenode));
			new->pos = pos;
			new->size = size;
			new->next = ptr->next;
			ptr->next = new;
			return pos;
		}

		ptr = ptr->next;
	}

	/* should not reach here if freelist is valid */
	return -1;
}
