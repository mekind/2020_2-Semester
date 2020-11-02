#include "file.h"

pagenum_t file_alloc_page(int fd) {
	Header_page Header;
	page_t Free;
	file_read_header_page(fd, &Header);
	pagenum_t ret = Header.Free_page;
	
	if (Header.Free_page) {
		file_read_page(fd, ret, &Free);
		Header.Free_page = Free.Next_Free;
	}
	else ret = Header.Number_of_pages++;

	file_write_header_page(fd, &Header);
	return ret;
}

void file_read_page(int fd, pagenum_t pagenum, page_t* dest) {
	if (pread(fd, dest, PAGE_SIZE, pagenum*PAGE_SIZE) == -1)
		perror("pread");
}

void file_read_header_page(int fd, Header_page* dest){
	if(pread(fd, dest, PAGE_SIZE, 0) == -1)
		perror("header read");
}

void file_write_page(int fd, pagenum_t pagenum, const page_t* src) {
	if (pwrite(fd, src, PAGE_SIZE, pagenum*PAGE_SIZE) == -1)
		perror("pwrite");
}

void file_write_header_page(int fd, const Header_page* src){
	if(pwrite(fd, src, PAGE_SIZE, 0)==-1)
		perror("write header");
}

int open_file(char *pathname){
	int fd = open(pathname, O_RDWR | O_CREAT, OPEN_MODE);
	
	if(fd ==-1) // 오류
		return fd;
	
	Header_page Header;
	memset(&Header, 0, sizeof(Header_page));
	file_read_header_page(fd, &Header);

	if(Header.Number_of_pages) // 그냥 열기
		return fd; 
	
	// 새로 만들어서 열기
	Header.Free_page = 0;
	Header.Root_page = 0;
	Header.Number_of_pages = 1;
	file_write_header_page(fd, &Header);

	return fd;
}

int close_file(int fd){
	return close(fd);
}



