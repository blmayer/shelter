#include "defs.h"
#include "methods.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    unsigned char rec[] = {'a', 'b', 'c', 0, 4, 0, 0, 0, 3, 'x', 'y', 'z'};
    unsigned char zero[] = {0};

    assert(keylen(rec) == 3);
    puts("keylen(rec) == 3 - PASSED");

    assert(keylen(zero) == 0);
    puts("keylen(zero) == 0 - PASSED");
}
