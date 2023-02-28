#ifndef DEFS
#define DEFS

#define VERSION "1.0.0"
#define MAX_DATA_SIZE 1024
#define DATA_DIR "data/"

#define DEBUG(args...) (if (debug) printf("[DEBUG] %s:%d ", __FILE__, __LINE__); printf(args))

enum type { 
	type_false = 0,
	type_true = 1,
	type_int = 2, 
	type_decimal = 3, 
	type_string = 4, 
	type_map = 5, 
	type_list = 6, 
	type_link = 7,
	type_key = 8,
	type_record = 9
};

enum operation { 
	op_get = 0, 
	op_put = 1, 
	op_update = 2, 
	op_del = 4
};

#endif
