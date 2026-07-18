# Shelter

A graph-oriented key-value database written in C. Records are stored in a
flat memory arena, linked together to form a traversable graph, and
persisted by writing the raw memory to disk.

See the full [design document](DESIGN.md) for wire formats and internals.


## Dependencies

- A C compiler (gcc or clang)
- GNU make


## Building

```sh
make          # debug build → bin/db
make release  # optimized static build
make test     # run the test suite
```


## Running

```sh
./bin/db
```

The server listens on **port 8080** by default. Set the `PORT` environment
variable to override. Set `DEBUG=1` to enable debug logging.

Two data files are created in the working directory:
- `data.bin` — the memory arena snapshot
- `index.bin` — the key→position index (text, append-only)


## Operations

Shelter communicates over TCP using a simple binary protocol. Each message
starts with a one-byte operation code followed by a payload. See `DESIGN.md`
for the full wire format.

| Operation | Code  | Description                                 |
|-----------|-------|---------------------------------------------|
| GET       | `'g'` | Retrieve a record by key                    |
| PUT       | `'p'` | Store a new record                          |
| UPDATE    | `'u'` | Replace an existing record (key must exist) |
| DELETE    | `'d'` | Remove a record by key                      |
| LINK      | `'l'` | Add a named link from one record to another |
| UNLINK    | `'n'` | Remove a named link                         |
| QUERY     | `'q'` | Graph traversal starting from a key         |

### Quick example with the client library

```c
#include "src/client.h"

int main(void) {
    int fd = shelter_connect("127.0.0.1", 8080);

    /* store two records */
    shelter_put(fd, "Peter", "greeting", "hello!");
    shelter_put(fd, "Alice", "greeting", "hi!");

    /* link them */
    shelter_link(fd, "Peter", "friend", "Alice");

    /* retrieve a record */
    unsigned char buf[1024];
    int n = shelter_get(fd, "Peter", buf, sizeof(buf));

    /* query: find all friends of Peter */
    unsigned char qbuf[4096];
    int qn = shelter_query(fd, "Peter", "friend", "*", qbuf, sizeof(qbuf));

    /* unlink and delete */
    shelter_unlink(fd, "Peter", "friend", "Alice");
    shelter_del(fd, "Alice");

    shelter_close(fd);
}
```


### Query operation

The query operation performs a graph traversal starting at a given key. Each
step follows links matching a field name and an optional value filter
(`"*"` for wildcard).

**Example:** Find all friends of Peter:

```
key: "Peter"
step: field = "friend", value = "*"
```

The response is a list of records reachable by following the matching links.
Multi-hop traversals are supported by chaining steps (e.g., friends of
friends).


## Testing

```sh
make test
```

Runs the full test suite covering: record helpers, allocator, trie index,
put/get, update, delete, link, unlink, and graph query.


## License

Distributed under the MIT License. See [LICENSE](LICENSE) for more
information.
