#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__
#include <stdint.h>
#include <unistd.h> 
#include<stdlib.h> // for malloc
#include <pthread.h> // thread 
#include <unordered_map> // hash table
#include <vector> // hash table

typedef struct lock_table_t lock_table_t;
typedef struct lock_table_t* ptr_lock_table_t;
typedef struct lock_t lock_t;
typedef struct lock_t* ptr_lock_t;

struct lock_table_t {
	/* NO PAIN, NO GAIN. */
	int table_id;
	int64_t key;
	int idx;
	ptr_lock_t head;
	ptr_lock_t tail;
};

struct lock_t{
	ptr_lock_t next;
	ptr_lock_t prev;
	ptr_lock_table_t sentinel;
    pthread_cond_t waiting_cond;
};

/* APIs for lock table */
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key);
int lock_release(lock_t* lock_obj);

ptr_lock_t make_new_waiting(ptr_lock_t next, ptr_lock_t prev, ptr_lock_table_t senti );
ptr_lock_table_t make_new_lock_table(int table_id, int64_t key, int idx);

#endif /* __LOCK_TABLE_H__ */
