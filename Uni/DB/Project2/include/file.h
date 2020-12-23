#ifndef __FILE_H__
#define __FILE_H__

//uint64_t
#include <stdint.h> 
// open 
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
//pread, pwrite, close
#include <unistd.h>
//error
#include <errno.h>
#include <stdlib.h> //malloc
#include <stdio.h>
#include <inttypes.h> // PRId64, PRIu64 , uint64 출력

#define OPEN_MODE 0666
#define PAGE_SIZE (size_t)4096
#define INTERNAL_ORDER 249
#define LEAF_ORDER 32
#define VALUE_SIZE 120

typedef uint64_t pagenum_t;
typedef long long my_key_t;

typedef struct Header_page {
	pagenum_t Free_page; //첫번째 아직 쓰이지 않은 페이지, 없으면 0
	pagenum_t Root_page; //root page 저장
	pagenum_t Number_of_pages; //현재 존재하는 페이지 수
	char Reserved[4072]; // size 조정
} Header_page;

typedef struct Record {
	my_key_t key;
	char value[VALUE_SIZE];
} Record;

typedef struct Index {
	my_key_t key;
	pagenum_t Next_Page;
} Index;

typedef struct page_t {
	union
	{
		pagenum_t Parent; // Leaf, Internal은 부모 페이지 저장
		pagenum_t Next_Free; // Free는 다음 Free 페이지 저장, Free는 여기만 초기화 한다.
	};	

	int Is_Leaf; // Leaf 여부 저장
	int Num_Keys; //key 개수 저장 
	char Reserved[104];

	pagenum_t Next_Page;
	//Leaf에서는 다음 Leaf 저장
	//Internal에서는 leftmost child page 저장

	union
	{
		Record Records[LEAF_ORDER - 1]; // Order 32
		Index Indexes[INTERNAL_ORDER - 1]; // Order 249
	};
} page_t;

int table; // 파일 디스크립터
extern int errno; // error 출력 

// free page
pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);


//read
void file_read_page(pagenum_t pagenum, page_t* dest);
void file_read_page_parent(pagenum_t pagenum, pagenum_t* dest);
void file_read_header_page();

//write
void file_write_page(pagenum_t pagenum, const page_t* src);
void file_write_page_parent(pagenum_t pagenum, pagenum_t * buffer);
void file_write_header_page();

//table
int open_table(char *pathname);
int close_table();

#endif


