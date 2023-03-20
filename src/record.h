/*
 *  This is the function that sends the file requested in a response, it takes
 *  two arguments: the client's connection and the file requested in the GET
 *  message.
 */

#ifndef RECORD_H
#define RECORD_H

#include <stddef.h>

char getop(unsigned char *rec);

size_t datalen(unsigned char *data);

size_t reclen(unsigned char *rec);

unsigned char *addlink(unsigned char *from, char *field, unsigned char *addr);

void printrec(unsigned char *rec);

size_t getlen(unsigned char *data);

enum type rectype(unsigned char *rec);

int typelen(unsigned char *data);

enum type gettype(unsigned char *data);

size_t keylen(unsigned char *rec);

unsigned char *next(unsigned char *rec);

unsigned char *data(unsigned char *rec);

#endif
