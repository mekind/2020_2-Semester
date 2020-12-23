#include "file.h"

Header_page *Header;

pagenum_t file_alloc_page() {
	pagenum_t ret = Header->Free_page;
	
	if (Header->Free_page) 
		file_read_page_parent(ret, &Header->Free_page); 
	else ret = Header->Number_of_pages++;

	file_write_header_page();
	return ret;
}

void file_free_page(pagenum_t pagenum) {
	if (pwrite(table, &(Header->Free_page), sizeof(pagenum_t), pagenum*PAGE_SIZE) == -1) 
		perror("free_page");
	sync();
}

void file_read_page(pagenum_t pagenum, page_t* dest) {

	if (pread(table, dest, PAGE_SIZE, pagenum*PAGE_SIZE) == -1)
		perror("pread");
}

void file_read_page_parent(pagenum_t pagenum, pagenum_t* dest) {

	if (pread(table, dest, sizeof(pagenum_t), pagenum*PAGE_SIZE) == -1) 
		perror("pread");
}

void file_write_page(pagenum_t pagenum, const page_t* src) {

	if (pwrite(table, src, PAGE_SIZE, pagenum*PAGE_SIZE) == -1)
		perror("pwrite");
	sync();
}

void file_write_page_parent(pagenum_t pagenum, pagenum_t * buffer) {

	if (pwrite(table, buffer, sizeof(pagenum_t), PAGE_SIZE * pagenum) == -1)
		perror("pwrite");
	sync();
}

void file_read_header_page() {
	if (pread(table, Header, PAGE_SIZE,0) == -1) 
		perror("pread header");
}

void file_write_header_page() { 

	if (pwrite(table, Header, PAGE_SIZE,0) == -1) 
		perror("pwrite header ");
	sync();
}

int open_table(char *pathname) {
	table = open(pathname, O_RDWR | O_CREAT, OPEN_MODE);
	if(table==-1) 
		perror("open");
	return table;
}

int close_table() {
	int ret = close(table);
	if (ret == -1) 
		perror("close");
	return ret;
}
