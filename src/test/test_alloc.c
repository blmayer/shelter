#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "defs.h"
#include "alloc.h"

struct freenode *freelist;
char debug = 1;

int main(void) {
	DEBUG("test alloc started\n");

	freelist = malloc(sizeof(struct freenode));

	/* pretend we have a free block of 200 bytes */
	freelist->pos = 0;
	freelist->size = 200;
	
	/* ask for 50 bytes */
	DEBUG("getting pos\n");
	int pos = getpos(50);
	assert(pos == 0);

	/* freenode must point to 50 now */
	assert(freelist->pos == 50);
	assert(freelist->size == 150);
	puts("getpos(50) - PASSED");


	/* ask for 30 bytes */
	DEBUG("getting pos\n");
	int pos2 = getpos(30);
	assert(pos2 == 50);

	/* freenode must point to 50 now */
	assert(freelist->pos == 80);
	assert(freelist->size == 120);
	puts("getpos(30) - PASSED");

	/* freelist is now
	 * +-----------------------------------------+
	 * |XXXXXXXXXXXXXXXXX                        |
	 * +-----------------------------------------+
	 * 0           50    80                      200
	 * 
	 * pos = 80, size = 120
	 */

	/* now we free the first part of 50 */
	freepos(0, 50);

	/* freelist is now
	 * +-----------------------------------------+
	 * |           XXXXXX                        |
	 * +-----------------------------------------+
	 * 0           50    80                      200
	 * so 2 nodes:
	 * pos = 0, size = 50
	 * pos = 80, size = 120
	 * */
	assert(freelist->pos == 0);
	assert(freelist->size == 50);

	assert(freelist->next->pos == 80);
	assert(freelist->next->size == 120);
	puts("freepos(0, 50) - PASSED");

}

