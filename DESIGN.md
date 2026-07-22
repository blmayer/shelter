# Shelter — Design Document

Shelter is a graph-oriented key-value database written in C. Objects are
identified by a unique key, can hold arbitrary typed data, and can be linked
to other objects to form a traversable graph.

The entire dataset lives in a flat memory arena (`mem[]`). Persistence is
achieved by writing the raw memory contents to a binary file (`data.bin`)
and recording key→position mappings in a text index (`index.bin`). Because
the memory layout *is* the disk layout, saving and loading are essentially
`memcpy`.

---

## Architecture

```
  Client (TCP)
      │
      ▼
 ┌──────────┐    ┌──────────┐    ┌──────────────┐
 │ handler.c │───►│ methods.c│───►│ mem[100 MiB] │
 │  (parse)  │    │  (logic) │    │  (arena)     │
 └──────────┘    └────┬─────┘    └──────┬───────┘
                      │                 │
                 ┌────▼─────┐     ┌─────▼──────┐
                 │  map.c   │     │  alloc.c   │
                 │  (trie   │     │  (free-list│
                 │   index) │     │   alloc)   │
                 └──────────┘     └────────────┘
```

| Component   | File          | Purpose                                      |
|-------------|---------------|----------------------------------------------|
| Server      | `main.c`      | TCP accept loop, fork-per-connection          |
| Handler     | `handler.c`   | Parse wire messages, dispatch to methods      |
| Methods     | `methods.c`   | Business logic: get, put, update, delete, link, unlink, query |
| Record      | `record.c`    | Binary record encoding, addlink, rmlink       |
| Map (trie)  | `map.c`       | In-memory trie index: key → arena position    |
| Allocator   | `alloc.c`     | Free-list arena allocator                     |


## Data model

### Type codes

Each datum starts with a one-byte type tag:

| Code | Char | Type     | Has length field? |
|------|------|----------|-------------------|
| `'f'`| `f`  | false    | No (0 bytes data) |
| `'t'`| `t`  | true     | No (0 bytes data) |
| `'i'`| `i`  | int      | No (4 bytes data) |
| `'d'`| `d`  | decimal  | No (4 bytes data) |
| `'s'`| `s`  | string   | Yes               |
| `'m'`| `m`  | map      | Yes               |
| `'l'`| `l`  | list     | Yes               |
| `'L'`| `L`  | link     | Yes               |
| `'k'`| `k`  | key      | Yes               |
| `'r'`| `r`  | record   | Yes               |


### Record layout

A record is a flat byte sequence: a type byte, a 4-byte little-endian
length, and then consecutive typed fields.

```
byte 0     1..4              5 ...
+------+-------------------+-----------------------------------+
| 'r'  | total_data_len    | field₁  field₂  ...  fieldₙ      |
+------+-------------------+-----------------------------------+
```

`total_data_len` counts all bytes after itself (i.e. `reclen = 1 + total_data_len`).

Every record **must** start with a `type_key` field that contains the
record's unique key string:

```
byte 0    1..4          5..5+n
+-----+----------+--------------+
| 'k' | key_len  | key bytes    |
+-----+----------+--------------+
```


### Variable-length fields

Strings, keys, links, maps, and lists all follow the pattern:

```
+------+----------+------- ... ------+
| type | data_len |    data bytes    |
+------+----------+------- ... ------+
  1 B     4 B LE      data_len bytes
```


### Integers and decimals

Fixed-size, 4 bytes little-endian, no length field:

```
+------+-----------+
| type | value(4B) |
+------+-----------+
```


### Booleans

Just the type byte, no data:

```
+------+
| type |
+------+
```


### Links

A link connects the current record to another record **by key**. The link's
data portion contains two sub-keys: the *field name* and the *target key*.

```
+-----+----------+-----+---------+-------+-----+---------+--------+
| 'L' | link_len | 'k' | fld_len | field | 'k' | tgt_len | target |
+-----+----------+-----+---------+-------+-----+---------+--------+
```

Example: a link with field "friend" pointing to key "Alice":
```
'L' 16,0,0,0  'k' 6,0,0,0 "friend"  'k' 5,0,0,0 "Alice"
```

On follow (query, etc.), the target key is resolved through the in-memory
index (`key → arena position`) to locate the current record bytes.


### Link target identity (decision — do not casually change)

**Links store the target's key string, not an arena address and not a
stable integer handle.**

This is intentional. Update, link, and unlink may free a record's old arena
slot and allocate a new one, so the byte offset of a record is not stable.
Anything that embeds a raw `pos` in a link will dangle after the target
moves (or worse, point at a later occupant of that slot).

Alternatives that were considered and rejected for now:

| Approach | Why not (for Shelter today) |
|----------|-----------------------------|
| **Arena address in the link** | Breaks when the target is reallocated; silent corruption risk. |
| **Stable handles** (`key → handle → pos`) | Correct and compact, but needs a durable handle table, alloc/reuse (or generation) policy, and more put/update/delete bookkeeping. Worth revisiting only if link size or hop cost becomes a measured problem. |
| **Reverse index + rewrite addresses on move** | Keeps pointer-sized links correct, but every move must patch all inbound edges; extra structure to maintain and persist. |
| **Forwarding stubs at old positions** | Works until compact; freelist and GC become more complex. |

**Prefer keys unless there is a concrete performance or size reason to pay
for handles (or reverse edges).** If that day comes, change the format in
one place, migrate on-disk data, and update this section — do not mix key
targets and address targets in the same database.


## Operations

### Wire protocol

The server listens on a TCP port (default 8080, configurable via the `PORT`
environment variable). Each message is a single byte identifying the
operation, followed by operation-specific payload bytes.

```
+------+------ ... ------+
|  op  |    payload      |
+------+------ ... ------+
```

Operation codes (ASCII chars):

| Op    | Code | Payload                          | Response              |
|-------|------|----------------------------------|-----------------------|
| GET   | `'g'`| `key_field`                      | Full record, or `\0`  |
| PUT   | `'p'`| Full record                      | `'t'` / `'f'`        |
| UPDATE| `'u'`| Full record (key must exist)     | `'t'` / `'f'`        |
| DELETE| `'d'`| `key_field`                      | `'t'` / `'f'`        |
| LINK  | `'l'`| `from_key` `field_key` `to_key`  | `'t'` / `'f'`        |
| UNLINK| `'n'`| `from_key` `field_key` `to_key`  | `'t'` / `'f'`        |
| QUERY | `'q'`| `start_key` + steps              | `list` of records     |


### GET

Retrieve a record by key.

**Request:** `'g'` + `type_key` field  
**Response:** The full record bytes, or a single `\0` if not found.


### PUT

Store a new record. The record must contain a `type_key` field as its first
data field. The key is extracted and used as the lookup key.

**Request:** `'p'` + full record  
**Response:** `'t'` on success, `'f'` on failure.


### UPDATE

Replace an existing record. Fails if the key does not already exist.

**Request:** `'u'` + full record (with same key)  
**Response:** `'t'` on success, `'f'` on failure.


### DELETE

Remove a record by key.

**Request:** `'d'` + `type_key` field  
**Response:** `'t'` on success, `'f'` if key not found.


### LINK

Create a named link from one record to another. Both records must already
exist.

**Request:** `'l'` + three `type_key` fields: `from`, `field_name`, `to`  
**Response:** `'t'` on success, `'f'` on failure.


### UNLINK

Remove a named link from a record.

**Request:** `'n'` + three `type_key` fields: `from`, `field_name`, `to`  
**Response:** `'t'` on success, `'f'` on failure.


### QUERY (graph traversal)

Traverse the graph starting at a given key by following links that match
a field name and optional value filter. Each step narrows the result set
to the targets of matching links.

**Request:** `'q'` + `start_key` + one or more steps  
Each step: `type_key(field_name)` + `type_string(value_pattern)`  
Use `"*"` as value_pattern for a wildcard (match all targets).

**Response:** A `type_list` result:
```
+-----+-------+----------+----------+-----+
| 'l' | count | record₁  | record₂  | ... |
+-----+-------+----------+----------+-----+
```

**Example:** Find all friends of Peter:
```
key: "Peter"
step: field="friend", value="*"
```
Wire bytes:
```
'q'  'k' 5 "Peter"  'k' 6 "friend"  's' 1 "*"
```
Returns a list of all records that Peter links to via "friend".

Multi-hop traversal is supported by chaining steps. For example, finding
friends-of-friends:
```
'q'  'k' 5 "Peter"  'k' 6 "friend"  's' 1 "*"  'k' 6 "friend"  's' 1 "*"
```


## Persistence

### data.bin

The raw `mem[]` arena written to disk. On startup, if `data.bin` exists,
its contents are read directly into `mem[]`. On every write operation, the
modified region is flushed to the corresponding offset in the file.

### index.bin

A text file with one line per key:
```
<position> <key>
```

During operation the index is **append-only** (each put/update appends a
new line). On startup, every line is parsed into the in-memory trie; later
lines for the same key overwrite earlier positions. After loading, the
index file is **rewritten** from the trie so it only contains the current
state (one line per live key), preventing unbounded growth from historical
appends.


## Index (trie)

The in-memory index is a character-level trie. Each node stores a single
character, a position (or -1 if not a terminal), and an array of child
pointers. Lookup, insertion, and deletion are all O(key_length).


## Memory allocator

The arena allocator manages `mem[]` using a linked list of free blocks.
`getpos(size)` finds the first block large enough, carves out `size` bytes,
and returns the starting position. `freepos(pos, size)` returns a region
to the free list, merging with adjacent blocks when possible.
