#include "defs.h"
#include "methods.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    unsigned char false[] = {0};
    unsigned char string[] = {4, 3, 0, 0, 0, 'x', 'y', 'z'};
    unsigned char intdata[] = {2, 0, 7, 12, 99};
    unsigned char zerostring[] = {4, 0, 0, 0, 0};
    unsigned char zero[] = {};

    assert(datalen(string) == 3);
    puts("datalen(rec) - PASSED");

    assert(datalen(false) == 0);
    puts("datalen(nodata) - PASSED");

    assert(datalen(intdata) == 4);
    puts("datalen(zerodata) - PASSED");

    assert(datalen(zerostring) == 0);
    puts("datalen(zerostring) - PASSED");

    assert(datalen(zero) == 0);
    puts("datalen(zero) - PASSED");
}
