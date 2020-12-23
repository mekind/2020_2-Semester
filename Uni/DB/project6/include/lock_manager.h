#ifndef __LOCK_MANAGER_H__
#define __LOCK_MANAGER_H__
#include <stdint.h>
#include <unistd.h> 
#include<stdlib.h> // for malloc
#include <pthread.h> // thread 
#include <unordered_map> // hash table
#include <vector> // hash table
#include <set>//trx_id

typedef unsigned long long ull;

typedef struct wait_node_t  wait_node_t;
typedef struct lock_table_t lock_table_t;
typedef struct lock_table_t* ptr_lock_table_t;
typedef struct lock_t lock_t;
typedef struct lock_t* ptr_lock_t;

//lock manager 
struct lock_table_t {
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
	ptr_lock_t prev_trx;
	ptr_lock_t next_trx;
	int lock_mode;
    int is_sleep;
    int trx_id;
};

int init_lock_table();

int lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode, lock_t* &ret_lock);
int lock_release(int trx_id, int is_abort);
void lock_wait(lock_t* lock_obj, int trx_id);

int deadlock_detection(int trx_id, ptr_lock_table_t now_table, int lock_mode);
int dfs(int now, int target, std::set<int> &visit);




ptr_lock_t make_new_waiting(ptr_lock_t next, ptr_lock_t prev, ptr_lock_table_t senti, int lock_mode, int trx_id);
ptr_lock_table_t make_new_lock_table(int table_id, int64_t key, int idx);


#endif