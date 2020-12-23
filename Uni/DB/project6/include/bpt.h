#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//Project 2 
#include <inttypes.h> // PRId64, PRIu64 , uint64 ���
#include<string.h> // strcpy 
#include<string>
#include "buffer.h"

#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif


#ifndef VALUE_SIZE
#define VALUE_SIZE 120
#endif

#define NO_PAGE 0

typedef struct node* ptrnode;

typedef struct node {// for print
	pagenum_t key;
	ptrnode next;
}node;


// FUNCTION PROTOTYPES.

// Output and utility.

int cut(int length);

//find
pagenum_t db_find_leaf(int table_id, my_key_t key);
int db_find(int table_id, my_key_t key, char * ret_val);
int get_my_index(fnum_t parent, pagenum_t target);



// Insertion.
int db_insert(int table_id, my_key_t key, char *value);
int start_new_tree(int table_id, my_key_t key, char * value);
int insert_into_leaf(int table_id, pagenum_t leaf_num, my_key_t key, char* value);
int insert_into_leaf_after_splitting(int table_id, pagenum_t leaf_num, my_key_t key, char * value);
int insert_into_parent(int table_id, pagenum_t parent_num, pagenum_t left_num, my_key_t key, pagenum_t right_num);
int insert_into_new_root(int table_id, pagenum_t left_num, my_key_t key, pagenum_t right_num);
int insert_into_page(int table_id, pagenum_t n_num, int left_index, my_key_t key, pagenum_t right_num);
int insert_into_page_after_splitting(int table_id, pagenum_t old_page, int left_index,
	my_key_t key, pagenum_t right_num);

//Deletion
int db_delete(int table_id, my_key_t key);
int delete_entry(int table_id, pagenum_t page_num, my_key_t key);
fnum_t remove_entry_from_page(int table_id, pagenum_t page_num, my_key_t key);
int adjust_root(int table_id, fnum_t root);
int coalesce_pages(int table_id, pagenum_t n_num, pagenum_t neighbor_num, int n_index, my_key_t k_prime);

//for project5 
int db_find(int table_id, int64_t key, char* ret_val, int trx_id);
int db_update(int table_id, int64_t key, char* values, int trx_id);
int db_find(int table_id, int64_t key, char * ret_val);

#endif /*__BPT_H__*/


