/*
 *  This is the function that sends the file requested in a response, it takes
 *  two arguments: the client's connection and the file requested in the GET
 *  message.
 */

#ifndef METHODS_H
#define METHODS_H

#include <stddef.h>

int send_key(int conn, unsigned char *rec);

int put_key(int conn, unsigned char *rec);

char getop(unsigned char *rec);

size_t getlensize(unsigned char *rec);

size_t datalen(unsigned char *rec);

size_t getlen(unsigned char *rec);

unsigned char *next(unsigned char *rec);

unsigned char *data(unsigned char *rec);

#endif
