/*
 *  This is the function that sends the file requested in a response, it takes
 *  two arguments: the client's connection and the file requested in the GET
 *  message.
 */

#ifndef METHODS_H
#define METHODS_H

#include <stddef.h>

unsigned char *fetchkey(unsigned char *key);

int putkey(unsigned char *rec);

int dump(unsigned char *rec);

int linkobjs(char* from, char *field, char *to);

char getop(unsigned char *rec);

size_t getlensize(unsigned char *rec);

size_t datalen(unsigned char *rec);

size_t reclen(unsigned char *rec);

size_t getlen(unsigned char *rec);

enum type gettype(unsigned char *rec);

size_t keylen(unsigned char *rec);

unsigned char *next(unsigned char *rec);

unsigned char *data(unsigned char *rec);

#endif
