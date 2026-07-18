#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../map.h"
#include "../defs.h"
#include "../methods.h"
#include "../record.h"
#include "../alloc.h"

char debug = 0;
map addrs;
int dbfile;
int idxfile;
struct freenode *freelist;
unsigned char mem[100 * 1024 * 1024] = {};

static int passed = 0;
static int total = 0;

#define TEST(name) do { total++; printf("  %-50s ", name); } while(0)
#define PASS() do { passed++; puts("OK"); } while(0)

static void setup(void) {
	memset(mem, 0, sizeof(mem));
	init(&addrs);
	freelist = malloc(sizeof(struct freenode));
	freelist->pos = 0;
	freelist->size = 100 * 1024 * 1024;
	freelist->next = NULL;
}

/* ── put & get ── */
static void test_put_and_get(void) {
	puts("\n=== put & get ===");
	setup();

	unsigned char rec[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};

	TEST("putkey returns 0");
	assert(putkey(rec) == 0);
	PASS();

	TEST("fetchkey returns stored record");
	unsigned char *res = fetchkey("abc");
	assert(res != NULL);
	for (size_t i = 0; i < reclen(rec); i++) assert(rec[i] == res[i]);
	PASS();

	TEST("fetchkey for missing key returns NULL");
	assert(fetchkey("missing") == NULL);
	PASS();

	/* put a second record */
	unsigned char rec2[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'd', 'e', 'f', type_string, 3, 0, 0, 0, '1', '2', '3'};
	TEST("put second record, fetch both");
	assert(putkey(rec2) == 0);
	res = fetchkey("abc");
	assert(res != NULL);
	for (size_t i = 0; i < reclen(rec); i++) assert(rec[i] == res[i]);
	res = fetchkey("def");
	assert(res != NULL);
	for (size_t i = 0; i < reclen(rec2); i++) assert(rec2[i] == res[i]);
	PASS();
}

/* ── update ── */
static void test_update(void) {
	puts("\n=== update ===");
	setup();

	unsigned char rec[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'o', 'l', 'd'};
	assert(putkey(rec) == 0);

	TEST("update existing key");
	unsigned char rec2[] = {type_record, 24, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 7, 0, 0, 0, 'u', 'p', 'd', 'a', 't', 'e', 'd'};
	assert(updatekey(rec2) == 0);
	unsigned char *res = fetchkey("abc");
	assert(res != NULL);
	for (size_t i = 0; i < reclen(rec2); i++) assert(rec2[i] == res[i]);
	PASS();

	TEST("update nonexistent key returns -1");
	unsigned char rec3[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'z', 'z', 'z', type_string, 3, 0, 0, 0, 'n', 'o', 'p'};
	assert(updatekey(rec3) == -1);
	PASS();
}

/* ── delete ── */
static void test_delete(void) {
	puts("\n=== delete ===");
	setup();

	unsigned char rec[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};
	assert(putkey(rec) == 0);

	TEST("delete existing key");
	assert(delkey("abc") == 0);
	assert(fetchkey("abc") == NULL);
	PASS();

	TEST("delete nonexistent key returns -1");
	assert(delkey("nope") == -1);
	PASS();
}

/* ── link & addlink ── */
static void test_link(void) {
	puts("\n=== link ===");
	setup();

	unsigned char rec_a[] = {type_record, 22, 0, 0, 0, type_key, 5, 0, 0, 0, 'P', 'e', 't', 'e', 'r', type_string, 3, 0, 0, 0, 'h', 'i', '!'};
	unsigned char rec_b[] = {type_record, 22, 0, 0, 0, type_key, 5, 0, 0, 0, 'A', 'l', 'i', 'c', 'e', type_string, 3, 0, 0, 0, 'y', 'o', '!'};
	unsigned char rec_c[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'B', 'o', 'b', type_string, 3, 0, 0, 0, 's', 'u', 'p'};

	assert(putkey(rec_a) == 0);
	assert(putkey(rec_b) == 0);
	assert(putkey(rec_c) == 0);

	TEST("linkobjs adds link to record");
	assert(linkobjs("Peter", "friend", "Alice") == 0);
	unsigned char *res = fetchkey("Peter");
	assert(res != NULL);
	/* record should be larger now (original 23 + link overhead) */
	assert(reclen(res) > sizeof(rec_a));
	PASS();

	TEST("linkobjs second link");
	assert(linkobjs("Peter", "friend", "Bob") == 0);
	res = fetchkey("Peter");
	assert(res != NULL);
	PASS();

	TEST("addlink on heap record");
	unsigned char *hrec = malloc(sizeof(rec_a));
	memcpy(hrec, rec_a, sizeof(rec_a));
	unsigned char *linked = addlink(hrec, "to", "Alice");
	assert(linked != NULL);
	assert(reclen(linked) > sizeof(rec_a));
	free(linked);
	PASS();
}

/* ── unlink ── */
static void test_unlink(void) {
	puts("\n=== unlink ===");
	setup();

	unsigned char rec_a[] = {type_record, 22, 0, 0, 0, type_key, 5, 0, 0, 0, 'P', 'e', 't', 'e', 'r', type_string, 3, 0, 0, 0, 'h', 'i', '!'};
	unsigned char rec_b[] = {type_record, 22, 0, 0, 0, type_key, 5, 0, 0, 0, 'A', 'l', 'i', 'c', 'e', type_string, 3, 0, 0, 0, 'y', 'o', '!'};

	assert(putkey(rec_a) == 0);
	assert(putkey(rec_b) == 0);
	assert(linkobjs("Peter", "friend", "Alice") == 0);

	size_t linked_len = reclen(fetchkey("Peter"));

	TEST("unlinkobjs removes the link");
	assert(unlinkobjs("Peter", "friend", "Alice") == 0);
	unsigned char *res = fetchkey("Peter");
	assert(res != NULL);
	assert(reclen(res) < linked_len);
	/* should be back to original size */
	assert(reclen(res) == sizeof(rec_a));
	PASS();

	TEST("unlinkobjs on missing link returns -3");
	assert(unlinkobjs("Peter", "friend", "Alice") == -3);
	PASS();
}

/* ── query (graph traversal) ── */
static void test_query(void) {
	puts("\n=== query ===");
	setup();

	/* Peter --friend--> Alice, Peter --friend--> Bob */
	unsigned char rec_p[] = {type_record, 22, 0, 0, 0, type_key, 5, 0, 0, 0, 'P', 'e', 't', 'e', 'r', type_string, 3, 0, 0, 0, 'h', 'i', '!'};
	unsigned char rec_a[] = {type_record, 22, 0, 0, 0, type_key, 5, 0, 0, 0, 'A', 'l', 'i', 'c', 'e', type_string, 3, 0, 0, 0, 'y', 'o', '!'};
	unsigned char rec_b[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'B', 'o', 'b', type_string, 3, 0, 0, 0, 's', 'u', 'p'};

	assert(putkey(rec_p) == 0);
	assert(putkey(rec_a) == 0);
	assert(putkey(rec_b) == 0);
	assert(linkobjs("Peter", "friend", "Alice") == 0);
	assert(linkobjs("Peter", "friend", "Bob") == 0);

	/*
	 * query: start at Peter, follow "friend" links with wildcard value.
	 * Wire: type_key(5)"Peter" + type_key(6)"friend" + type_string(1)"*"
	 */
	unsigned char qmsg[] = {
		type_key, 5, 0, 0, 0, 'P', 'e', 't', 'e', 'r',
		type_key, 6, 0, 0, 0, 'f', 'r', 'i', 'e', 'n', 'd',
		type_string, 1, 0, 0, 0, '*'
	};
	int qlen = sizeof(qmsg);

	TEST("query Peter->friend->* returns 2 results");
	unsigned char *result = query(qmsg, &qlen);
	assert(result != NULL);
	assert(result[0] == type_list);
	int count = *(int *)(result + 1);
	assert(count == 2);
	PASS();

	/* verify each result is a valid record */
	TEST("query results are valid records");
	unsigned char *r = result + 5;
	for (int i = 0; i < count; i++) {
		assert(r[0] == type_record);
		int rlen = reclen(r);
		assert(rlen > 0);
		r += rlen;
	}
	PASS();

	free(result);

	/* query with specific value: only Alice */
	unsigned char qmsg2[] = {
		type_key, 5, 0, 0, 0, 'P', 'e', 't', 'e', 'r',
		type_key, 6, 0, 0, 0, 'f', 'r', 'i', 'e', 'n', 'd',
		type_string, 5, 0, 0, 0, 'A', 'l', 'i', 'c', 'e'
	};
	int qlen2 = sizeof(qmsg2);

	TEST("query Peter->friend->Alice returns 1 result");
	result = query(qmsg2, &qlen2);
	assert(result != NULL);
	count = *(int *)(result + 1);
	assert(count == 1);
	PASS();

	free(result);
}

/* ── record helpers ── */
static void test_record_helpers(void) {
	puts("\n=== record helpers ===");

	TEST("datalen for type_false is 0");
	unsigned char f[] = {type_false};
	assert(datalen(f) == 0);
	PASS();

	TEST("datalen for type_true is 0");
	unsigned char t[] = {type_true};
	assert(datalen(t) == 0);
	PASS();

	TEST("datalen for type_int is 4");
	unsigned char i[] = {type_int, 42, 0, 0, 0};
	assert(datalen(i) == 4);
	PASS();

	TEST("datalen for type_string reads length field");
	unsigned char s[] = {type_string, 5, 0, 0, 0, 'h', 'e', 'l', 'l', 'o'};
	assert(datalen(s) == 5);
	PASS();

	TEST("datalen for type_key reads length field");
	unsigned char k[] = {type_key, 3, 0, 0, 0, 'a', 'b', 'c'};
	assert(datalen(k) == 3);
	PASS();

	TEST("datalen for type_link reads length field");
	unsigned char l[] = {type_link, 10, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0};
	assert(datalen(l) == 10);
	PASS();

	TEST("reclen for full record");
	unsigned char rec[] = {type_record, 20, 0, 0, 0, type_key, 3, 0, 0, 0, 'a', 'b', 'c', type_string, 3, 0, 0, 0, 'x', 'y', 'z'};
	assert(reclen(rec) == 21);
	PASS();
}

/* ── allocator ── */
static void test_alloc(void) {
	puts("\n=== allocator ===");
	setup();

	TEST("getpos returns 0 for first alloc");
	int p1 = getpos(50);
	assert(p1 == 0);
	PASS();

	TEST("getpos returns sequential positions");
	int p2 = getpos(30);
	assert(p2 == 50);
	int p3 = getpos(20);
	assert(p3 == 80);
	PASS();

	TEST("freepos adds freed block to freelist");
	freepos(50, 30);
	/* the freed block should appear somewhere in the freelist */
	struct freenode *node = freelist;
	int found = 0;
	while (node) {
		if (node->pos == 50 || (node->pos <= 50 && node->pos + node->size >= 80)) {
			found = 1;
			break;
		}
		node = node->next;
	}
	assert(found);
	PASS();
}

/* ── map (trie index) ── */
static void test_map(void) {
	puts("\n=== map (trie) ===");

	map m;
	init(&m);

	TEST("add and get");
	add(&m, "hello", 42);
	assert(get(&m, "hello") == 42);
	PASS();

	TEST("get missing key returns -1");
	assert(get(&m, "world") == -1);
	PASS();

	TEST("del removes key");
	del(&m, (unsigned char *)"hello");
	assert(get(&m, "hello") == -1);
	PASS();

	TEST("multiple keys");
	add(&m, "foo", 1);
	add(&m, "bar", 2);
	add(&m, "baz", 3);
	assert(get(&m, "foo") == 1);
	assert(get(&m, "bar") == 2);
	assert(get(&m, "baz") == 3);
	PASS();

	destroy(&m);
}

int main(void) {
	test_record_helpers();
	test_alloc();
	test_map();
	test_put_and_get();
	test_update();
	test_delete();
	test_link();
	test_unlink();
	test_query();

	printf("\n%d/%d tests passed\n", passed, total);
	return (passed == total) ? 0 : 1;
}

