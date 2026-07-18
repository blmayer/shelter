#ifndef METHODS_H
#define METHODS_H

#include <stddef.h>
#include "defs.h"

/* Key operations */
unsigned char *fetchkey(char *key);
int putkey(unsigned char *rec);
int updatekey(unsigned char *rec);
int delkey(char *key);

/* Link operations */
int linkobjs(char *from, char *field, char *to);
int unlinkobjs(char *from, char *field, char *to);

/* Query (graph traversal) — returns a heap-allocated result buffer */
unsigned char *query(unsigned char *msg, int *outlen);

/* Wire-payload helpers (payload is the message after the op byte) */
unsigned char *get_from_payload(unsigned char *payload);
int del_from_payload(unsigned char *payload);
int link_from_payload(unsigned char *payload);
int unlink_from_payload(unsigned char *payload);

/* Index persistence */
int writeindex(char key[MAX_KEY_SIZE], int pos);
int loadindex(void);

#endif
