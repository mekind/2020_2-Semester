#ifndef __FILE_H__
#define __FILE_H__

#include "trx_manager.h"

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
#include <string.h> //memset

#define OPEN_MODE 0666
#define PAGE_SIZE 4096
#define INTERNAL_ORDER 249
#define LEAF_ORDER 32
#define VALUE_SIZE 120

typedef uint64_t pagenum_t;
typedef int64_t my_key_t;

typedef struct Header_page {
	pagenum_t Free_page; //첫번째 아직 쓰이지 않은 페이지, 없으면 0
	pagenum_t Root_page; //root page 저장
	pagenum_t Number_of_pages; //현재 존재하는 페이지 수
	char Reserved[4072]; // size 조정
} header_t;

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
	char Reserved1[8];
	uint64_t pLSN;
	char Reserved2[88];

	pagenum_t Next_Page;
	//Leaf에서는 다음 Leaf 저장
	//Internal에서는 leftmostdd child page 저장

	union
	{
		Record Records[LEAF_ORDER - 1]; // Order 32
		Index Indexes[INTERNAL_ORDER - 1]; // Order 249
	};
} page_t;

extern int errno;

// free page
pagenum_t file_alloc_page(int fd);
void file_free_page(int fd, pagenum_t pagenum);

//read
void file_read_page(int fd, pagenum_t pagenum, page_t* dest);
void file_read_header_page(int fd, Header_page* dest);

//write
void file_write_page(int fd, pagenum_t pagenum, const page_t* src);
void file_write_header_page(int fd, const Header_page* src);

//file
int open_file(char *pathname);
int close_file(int fd);

#endif


