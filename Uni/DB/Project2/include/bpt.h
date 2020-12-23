#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//Project 2 
#include <inttypes.h> // PRId64, PRIu64 , uint64 Ãâ·Â
#include<string.h> // strcpy 
#include "file.h"

#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

#ifndef VALUE_SIZE
#define VALUE_SIZE 120
#endif

#define NO_PAGE (pagenum_t)0
#define SUCCESS 0
#define FAIL 1

typedef struct node* ptrnode;

typedef struct node {// for print
	pagenum_t key;
	ptrnode next;
}node;
	
// FUNCTION PROTOTYPES.

// Output and utility.

void license_notice( void );
void usage_1( void );
void usage_2( void );

int cut(int length);
int path_to_root(pagenum_t pagenum);
void enqueue(pagenum_t pnum);
pagenum_t dequeue( void );

//print
void print_tree();
void print_leaves();
void print_page(pagenum_t p);
void print_header();



//find
pagenum_t db_find_leaf(my_key_t key);
int db_find(my_key_t key, char * ret_val);
int get_my_index(page_t * parent, pagenum_t target);

// Insertion.
int db_insert(my_key_t key, char *value);
int start_new_tree(my_key_t key, char * value);
int insert_into_leaf(pagenum_t leaf_num, page_t leaf, my_key_t key, char* value);
int insert_into_leaf_after_splitting(pagenum_t leaf_num, page_t leaf, my_key_t key, char * value);
int insert_into_parent(pagenum_t parent_num, pagenum_t left_num, my_key_t key, pagenum_t right_num);
int insert_into_new_root(pagenum_t left_num, my_key_t key, pagenum_t right_num);
int insert_into_page(pagenum_t n_num, page_t n, int left_index, my_key_t key, pagenum_t right_num);
int insert_into_page_after_splitting(pagenum_t old_page_num, page_t old_page, int left_index,
	my_key_t key, pagenum_t right_num);

//Deletion
int db_delete(my_key_t key);
int delete_entry(pagenum_t page_num, my_key_t key);
page_t remove_entry_from_page(pagenum_t page_num, page_t page, my_key_t key);
int adjust_root(pagenum_t root_num, page_t root);
int coalesce_pages(pagenum_t n_num, page_t n, pagenum_t neighbor_num, 
	page_t neighbor, int n_index, my_key_t k_prime);

// table

int open_table_in_memory(char *pathname);
int close_table_in_memory();
#endif /*__BPT_H__*/



