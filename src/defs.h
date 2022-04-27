#ifndef DEFS
#define DEFS

#define VERSION "1.0.0"
#define MAX_DATA_SIZE 1024
#define DATA_DIR "data/"

enum operation { get = 10, put, update, del };
enum type { string = 3, integer, boolean, decimal, map, list, key };
enum value { f, t, zero };

#endif