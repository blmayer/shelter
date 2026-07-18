/*
 * Shelter client library.
 *
 * Provides helpers to connect to a shelter server, build correctly
 * encoded records and queries, send them, and read the response.
 *
 * Usage:
 *   int fd = shelter_connect("127.0.0.1", 8080);
 *   shelter_put(fd, "mykey", "field", "value");
 *   int n = shelter_get(fd, "mykey", buf, sizeof(buf));
 *   shelter_close(fd);
 */

#ifndef SHELTER_CLIENT_H
#define SHELTER_CLIENT_H

#include <stddef.h>

/* Connect to a shelter server. Returns the socket fd, or -1 on error. */
int shelter_connect(const char *host, int port);

/* Close the connection. */
void shelter_close(int fd);

/*
 * PUT — store a record with a single string field.
 * Returns 1 on success (server replied 't'), 0 on failure.
 */
int shelter_put(int fd, const char *key, const char *field, const char *value);

/*
 * PUT (raw) — store an arbitrary pre-built record.
 * `rec` must be a valid shelter record (type_record + len + fields).
 * Returns 1 on success, 0 on failure.
 */
int shelter_put_raw(int fd, const unsigned char *rec, size_t reclen);

/*
 * GET — retrieve a record by key.
 * Writes the raw record bytes into `buf` (up to `bufsz` bytes).
 * Returns the number of bytes received, or -1 on error / not found.
 */
int shelter_get(int fd, const char *key, unsigned char *buf, size_t bufsz);

/*
 * UPDATE — replace an existing record (key must already exist).
 * Takes a single string field for convenience.
 * Returns 1 on success, 0 on failure.
 */
int shelter_update(int fd, const char *key, const char *field, const char *value);

/*
 * DELETE — remove a record by key.
 * Returns 1 on success, 0 if key was not found.
 */
int shelter_del(int fd, const char *key);

/*
 * LINK — create a named link: from --field--> to.
 * Returns 1 on success, 0 on failure.
 */
int shelter_link(int fd, const char *from, const char *field, const char *to);

/*
 * UNLINK — remove a named link.
 * Returns 1 on success, 0 on failure.
 */
int shelter_unlink(int fd, const char *from, const char *field, const char *to);

/*
 * QUERY — single-step graph traversal.
 * Start at `start_key`, follow links named `field` whose target matches
 * `value` (use "*" for wildcard).
 *
 * Writes the raw result list into `buf` (type_list + count + records).
 * Returns total bytes received, or -1 on error.
 * The record count can be read as *(int*)(buf+1).
 */
int shelter_query(int fd, const char *start_key, const char *field,
                  const char *value, unsigned char *buf, size_t bufsz);

/*
 * Build a shelter record with one key and one string field.
 * Writes into `buf` and returns the total byte length, or -1 if buf is
 * too small.
 *
 * Layout: type_record(1) + len(4) + type_key(1) + keylen(4) + key
 *         + type_string(1) + vallen(4) + value
 */
int shelter_build_record(const char *key, const char *field, const char *value,
                         unsigned char *buf, size_t bufsz);

#endif