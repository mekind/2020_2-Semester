#ifndef __TRX_MANAGER_H__
#define __TRX_MANAGER_H__
#include "lock_manager.h"
#include <stdint.h>
#include <unistd.h> 
#include<stdlib.h> // for malloc
#include <pthread.h> // thread 
#include <unordered_map> // hash table
#include <vector> // hash table
#include<string.h>
#include<string>


typedef unsigned long long ull;
typedef struct trx_t trx_t;
typedef struct trx_table_t trx_table_t;
typedef struct my_log_t my_log_t;

//transaction manager

struct my_log_t {
	uint64_t pnum;
	int table_id;
	int64_t key;
	char val[120];
};


struct trx_t {
	lock_t* head;
    lock_t* tail;
	std::set<int> wait_for; // 같은 거 있을까봐 set으로 함
	std::vector<my_log_t> my_log;
	uint64_t prev_LSN;
	pthread_mutex_t trx_lock;
};

struct trx_table_t {
	ull cnt;
	std::unordered_map< int, trx_t*> trx_list;
};

int trx_begin(void);
int trx_commit(int trx_id);
int trx_destroy(int trx_id);
int make_log_in_trx(int trx_id, const char* val, lock_t* obj, uint64_t pnum);

trx_t* get_trx(int trx_id);

#endif