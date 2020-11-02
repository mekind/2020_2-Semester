#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "file.h"
#include<unordered_map>
#include <stdint.h>

#define PATHNAME_SIZE 25 
#define TABLE_TOTAL 15 

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
} frame_t;

typedef struct Table_Info{
	int fd[TABLE_TOTAL];
	char pathname[TABLE_TOTAL][PATHNAME_SIZE];
	int cnt;
} table_t;

typedef int fnum_t;

int init_db(int num_buf);
int shutdown_db();
// Allocate the buffer pool

int open_table(char *pathname);
int close_table(int table_id);
bool is_open(int table_id);
// table 


void buffer_write_frame(int table_id, int fnum);
fnum_t buffer_read_frame(int table_id, pagenum_t pnum);
fnum_t buffer_alloc_frame();
fnum_t buffer_alloc_page(int table_id);
// Bufffer manage
void buffer_unpin(fnum_t fnum);
void LRU_change(fnum_t fnum);

#endif

