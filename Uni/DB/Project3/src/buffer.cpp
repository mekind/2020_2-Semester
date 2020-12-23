#include "buffer.h"
#include<stdlib.h>

#define SUCCESS 0
#define FAIL 1
#define mp std::unordered_map<pagenum_t, fnum_t>::value_type


frame_t *Buffer = NULL;
table_t Table;
std::unordered_map<pagenum_t, fnum_t> hash_table[11];


int init_db(int num_buf){
	if(Buffer){
		printf("Already init\n");
		return FAIL;
	}
	
	Buffer = (frame_t *)malloc(sizeof(frame_t) * (num_buf + 1));
	memset(Buffer, 0, sizeof(frame_t) * (num_buf + 1));
	memset(&Table, 0, sizeof(table_t));
	Buffer[0].size=num_buf;

	return SUCCESS;
}

int shutdown_db(){
	if(!Buffer){
		printf("Error : Empty db\n");
		return FAIL;
	}

	int i;
	for (i = 1; i <= 10; i++)
		close_table(i);
	
	free(Buffer);
	Buffer=NULL;
	memset(&Table, 0, sizeof(table_t));
	
	return SUCCESS;
}

int open_table(char *pathname){
	
	int i;

	for(i=1;i<=10;i++){ 
		int check = strcmp(pathname, Table.pathname[i]); 

		if (check == 0 && Table.fd[i] > 0) { // 이미 열림
//			printf("Already open %s table\n", pathname);
			return i;
		}
		else if(check == 0){ // 한 번더 열기
//			printf("OPEN SUCCESS 2 \n");
			Table.fd[i]=open_file(pathname);
			return i;
		}
	}
	
	if (Table.cnt >= 10) { // 10개 이상 안 엶
//		printf("Already open 10 tables\n");
		return -1;
	}

//	printf("OPEN SUCCESS \n");

	i = ++Table.cnt; // 없으면 추가
	Table.fd[i] = open_file(pathname);
	strcpy(Table.pathname[i], pathname);

	return i;
}

int close_table(int table_id){
	
	int fd = Table.fd[table_id];

	if (fd < 0)
		return FAIL;

	auto iter = hash_table[table_id].begin();

	for(iter; iter!=hash_table[table_id].end();iter++){
		int now = iter->second;
		buffer_write_frame(table_id, now);
	}

	Table.fd[table_id] = -1;
	hash_table[table_id].clear();
	close_file(fd);

	return SUCCESS;
}

void buffer_write_frame(int table_id, int fnum){
	if (Buffer[fnum].is_dirty == 0 || table_id <= 0) {
		memset(&Buffer[fnum].page, 0, sizeof(page_t));
		return;
	}
		

	++Buffer[fnum].is_pinned;
	// is_dirty	
	if(Buffer[fnum].pnum) // leaf, internal, free
		file_write_page(Table.fd[table_id], Buffer[fnum].pnum, &Buffer[fnum].page);
	else //header 일때
		file_write_header_page(Table.fd[table_id], &Buffer[fnum].hpage);

	memset(&Buffer[fnum].page, 0, sizeof(page_t));
	Buffer[fnum].is_dirty = 0;
	--Buffer[fnum].is_pinned;
}

fnum_t buffer_read_frame(int table_id, pagenum_t pnum){
	fnum_t fnum;
	auto iter = hash_table[table_id].find(pnum);
	
	if (iter != hash_table[table_id].end()) { // 해당 페이지가 이미 존재할 때
		fnum = iter->second;
		LRU_change(fnum);
		++Buffer[fnum].is_pinned;
		return fnum;
	}
	
	// 존재하지 않는다면 	

	fnum = buffer_alloc_frame();
	Buffer[fnum].table_id = table_id;
	Buffer[fnum].pnum = pnum;
	hash_table[table_id].insert(mp( pnum, fnum ));
	
	if(pnum)
		file_read_page(Table.fd[table_id], pnum, &Buffer[fnum].page);
	else
		file_read_header_page(Table.fd[table_id], &Buffer[fnum].hpage);

	LRU_change(fnum);
	return fnum;
}


void LRU_change(fnum_t now){
	fnum_t &head = Buffer[0].next;
	fnum_t &tail = Buffer[0].prev;
	
	if(head ==0){
			head = tail = now;
			Buffer[now].prev = Buffer[now].next = 0;
			return;
	}

	if(tail==now)
		return;
	
	fnum_t &prev = Buffer[now].prev;
	fnum_t &next = Buffer[now].next;

	if(prev || next){
		++Buffer[prev].is_pinned;
		++Buffer[next].is_pinned;

		Buffer[prev].next=Buffer[now].next;
		Buffer[next].prev=Buffer[now].prev;

		buffer_unpin(prev);
		buffer_unpin(next);
	}

	++Buffer[tail].is_pinned;
	Buffer[tail].next = now;
	Buffer[tail].is_dirty = 1;
	--Buffer[tail].is_pinned;

	Buffer[now].prev =tail;
	Buffer[now].next =0;
	tail= now;
}

fnum_t buffer_alloc_frame(){
	fnum_t start = Buffer[0].next;

	if (Buffer[0].now < Buffer[0].size) // 여유가 있음
		start = ++Buffer[0].now;
	else { //LRU Policy

		while (Buffer[start].is_pinned){
			start = Buffer[start].next;
		//	printf("%d now\n",start);
		}

		buffer_write_frame(Buffer[start].table_id, start); //eviction
		hash_table[Buffer[start].table_id].erase(Buffer[start].pnum);
	}

	++Buffer[start].is_pinned;
	return start;
}

fnum_t buffer_alloc_page(int table_id) {
	
	fnum_t Header = buffer_read_frame(table_id, 0);
	
	fnum_t Free;
	pagenum_t Free_pnum = Buffer[Header].hpage.Free_page;

	if (Free_pnum) {
		Free = buffer_read_frame(table_id, Free_pnum);
		Buffer[Header].hpage.Free_page = Buffer[Free].page.Next_Free;
		++Buffer[Free].is_pinned;
	}
	else {
		Free = buffer_alloc_frame();
		Free_pnum = Buffer[Header].hpage.Number_of_pages++;
	}
	
	hash_table[table_id].insert(mp(Free_pnum, Free ));
	buffer_unpin(Header);
	Buffer[Free].table_id = table_id;
	Buffer[Free].pnum = Free_pnum;	
	LRU_change(Free);

	return Free;
}


void buffer_unpin(fnum_t fnum) {
	--Buffer[fnum].is_pinned;
	Buffer[fnum].is_dirty = 1;
}

bool is_open(int table_id) {
	if (Table.fd[table_id] < 0 || Table.cnt < table_id)
		return false;

	return true;
}


