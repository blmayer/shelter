# Data

Each object is stored in memory and in disk. Converting one to the other
when starting up or dumping objects.

Here each object represents one unique datum that is stored, to indentify one
object the key field is used. Objects can be linked to each other if they are
related in some way, this is translated to memory representation as a pointer,
and to disk as a symlink.


## Fields

There are some constraints regarding objects, as they are flexible data some
extra care must be taken.


### Key field

Objects are required to have a key field to uniquely identify it. This field
is special and has the type "key", with its own representations.


### Links

A relation can be added between two or more objects, they can represent any
one to many relation as in real life.


### Data

Likewise, objects contain data in any form, strings, lists, maps, or numbers.
Objects can be any of these or contain them.


## Filesystem representation

For the disk we use the filesystem to designate objects by its key, objects
are saved using the key value as the path, e.g. the object with key "user/x"
would be saved in the folder "user" as a file "x".


## Memory representation

An object is just an array of bytes, with 2 logical parts: key and data. As
C does not have dynamicaly sized arrays, an object is just []char.

To separate the key part from the data part a zero byte is used. The data
part starts with a 4 bytes integer that represents its total size, and then
the actual data that the user wants to store.

```
                             bytes
0      1       2       3       4       5       6       7       8
+------+-------+-------+-------+-------+-------+-------+-------+
< key of variable size >  \0   < -------- data length -------- >
+------+-------+-------+-------+-------+-------+-------+-------+
< --------------------------- data --------------------------- >
+------+-------+-------+-------+-------+-------+-------+-------+
```

Now the data part has its own components, each data type has one byte to
indicate its type, and, depending on the type 4 bytes to indicate the length.
Fixed sizes like int, float, double and bool do not need the length. In the
next examples some data types are represented:


### numbers

Number like, int, floats are fixed size, their difference is the type byte:
4: integer and 5:float.

```
                             bytes
0      1       2       3       4       5       6       7       8    ...
+------+-------+-------+-------+-------+-------+-------+-------+---
  type < number data in little endian  >
+------+-------+-------+-------+-------+-------+-------+-------+---
```


### String

```
                             bytes
0      1       2       3       4       5       6       7       8    ...
+------+-------+-------+-------+-------+-------+-------+-------+---
    3  < ----- length of string ------ |  string data in ASCII ---- ...
+------+-------+-------+-------+-------+-------+-------+-------+---
```


### booleans and zero

Booleans and zero take just one byte, the type: 0:false, 1: true and 2:zero.
```
                             bytes
0      1       2       3       4       5       6       7       8    ...
+------+-------+-------+-------+-------+-------+-------+-------+---
| type |
+------+-------+-------+-------+-------+-------+-------+-------+---
```


# System design

This program receives a stream of instructions on objects
and the goal is to execute the commands in the filesystem.


```
───►   key X              x
       field f            ┌──────────┐
       set = 10           │type map  │
                          │field next│
                          │ type ptr │
                          │ key y    │
       key x              └──────────┘
       field next         y
       go                 ┌──────────┐
       set ver = 2        │ver = 2   │
                          │          │
          │               │          │
          │               │          │
          │               └──────────┘
          └───────►       z             za            ab
                          ┌──────────┐  ┌──────────┐  ┌──────────┐
   Instructions are       │          │  │          │  │          │
   parsed and executed    │          │  │          │  │          │
                          │          │  │          │  │          │
                          │          │  │          │  │          │
                          └──────────┘  └──────────┘  └──────────┘
                          a             as            at
                          ┌──────────┐  ┌──────────┐  ┌──────────┐
                          │          │  │          │  │          │
                          │          │  │          │  │          │
                          │          │  │          │  │          │
                          │          │  │          │  │          │
                          └──────────┘  └──────────┘  └──────────┘
```
