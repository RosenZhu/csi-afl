

// #define NUM_EDGE_FILE "num_edges.txt"

// #define COND_ADDR_ID "cond_addr_ids.txt"
// #define COND_NOT_ADDR_ID "cond_not_addr_id.txt"
// #define NO_JUMP_ADDR_ID "no_jump_addr_id.txt"
// #define UNCOND_JUMP_ADDR_ID "uncond_jump_addr_id.txt"

// #define INDIRECT_ADDR_ID "indirect_addr_ids.txt"

// //mark file for pre-determined edges
// #define PATH_MARKS "path_marks.txt"
// mark file for indirect edges
//#define INDIRECT_MARKS "indirect_marks.txt"

// #define BASE_INDIRECT           3
// //bytes for checksum for path id
// #define SIZE_CKSUM_PATH         (1 << 7)
// #define BYTES_CKSUM_PATH        (4 * SIZE_CKSUM_PATH)

#define BYTES_FLAGS     (1 << 19)

//bytes for recording the flags of loops
#define FLAG_LOOP   1

// give loops more air time
#define LOOP_TIME   8

#define BLOCK_ADDR_ID   "addr_ids.txt"


#define BLOCK_EXIT 66 //exit(BLOCK_EXIT)
