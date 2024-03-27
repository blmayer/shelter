#ifndef DEFS
#define DEFS

#define VERSION "1.0.0"
#define MAX_KEY_SIZE 128
#define MAX_DATA_SIZE 1024
#define DB_SIZE 100 * 1024 * 1024
#define DB_FILE "data.bin"
#define IDX_FILE "index.bin"

#define DEBUG(arg) if (debug) printf("[DEBUG] %s:%d %s", __FILE__, __LINE__, arg)

#define DEBUGF(format, args...) if (debug) {printf("[DEBUG] %s:%d: ", __FILE__, __LINE__); printf(format, args);}

enum type { 
	type_false = 'f',
	type_true = 't',
	type_int = 'i', 
	type_decimal = 'd', 
	type_string = 's', 
	type_map = 'm', 
	type_list = 'l', 
	type_link = 'L',
	type_key = 'k',
	type_record = 'r'
};

enum operation { 
	op_get = 'g', 
	op_put = 'p', 
	op_update = 'u', 
	op_del = 'd'
};

struct freenode {
	int pos;
	int size;
	struct freenode *next;
};

#endif
