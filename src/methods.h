/*
 *  This is the function that sends the file requested in a response, it takes
 *  two arguments: the client's connection and the file requested in the GET
 *  message.
 */

#ifndef METHODS_H
#define METHODS_H

int send_key(int conn, unsigned char *rec, unsigned int len);

#endif
