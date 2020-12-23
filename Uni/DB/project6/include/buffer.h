#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "file.h"
#include<unordered_map>
#include <stdint.h>
#include <pthread.h> // thread 
#include <string> // hash_key
#include <vector>


#define PATHNAME_SIZE 25 
#define mp std::unordered_map<pagenum_t, fnum_t>::value_type
typedef int fnum_t;

typedef struct Frame{
	union
	{
		page_t page;
		header_t hpage;
	};

	pagenum_t pnum;
	union
	{
		int table_id; 
		int now; // meta
	};

	char is_dirty;
	
	union
	{
		int is_pinned;
		int size; // meta
	};
	
	int prev; // meta, tail
	int next; // meta, head
	pthread_mutex_t page_latch;
} frame_t;

typedef struct Buffer_Manager_t {
	int fd[10];
	std::unordered_map<pagenum_t, fnum_t> hash_table[10];
} Buffer_Manager_t;

int init_db(int num_buf);
int init_db (int buf_num , int flag, int log_num , char* log_path , char* logmsg_path);
int shutdown_db();
// Allocate the buffer pool

int open_table(char *pathname);
int close_table(int table_id);
bool is_open(int table_id);
// table 
bool operator<(const Record &a, my_key_t b);
bool operator<(my_key_t b, const Index &a);


void buffer_write_frame(int table_id, int fnum);
fnum_t buffer_read_frame(int table_id, pagenum_t pnum);
fnum_t buffer_alloc_page(int table_id);
fnum_t buffer_alloc_frame();
// Bufffer manage
void LRU_change(fnum_t fnum);
void buffer_unpin(fnum_t fnum);
void buffer_unpin_no_dirt(fnum_t fnum);


//project 5

int trx_abort(int trx_id);
#endif

