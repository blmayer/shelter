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