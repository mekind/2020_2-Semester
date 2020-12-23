#include "buffer.h"
#include<stdlib.h>
#include<algorithm>
#define SUCCESS 0
#define FAIL 1

frame_t *Buffer = NULL;
Buffer_Manager_t Buffer_Manager;
pthread_mutex_t Buffer_Manager_latch = PTHREAD_MUTEX_INITIALIZER;

int init_db(int num_buf){
	pthread_mutex_lock(&Buffer_Manager_latch);
	if(Buffer){
		//printf("Already init\n");
		pthread_mutex_unlock(&Buffer_Manager_latch);
		return FAIL;
	}
	
	Buffer = (frame_t *)malloc(sizeof(frame_t) * (num_buf + 1));
	memset(Buffer, 0, sizeof(frame_t) * (num_buf + 1));
	Buffer[0].size=num_buf;

	pthread_mutex_unlock(&Buffer_Manager_latch);
	return SUCCESS;
}

int shutdown_db(){
	pthread_mutex_lock(&Buffer_Manager_latch);
	if(!Buffer){
		//printf("Error : Empty db\n");
		pthread_mutex_unlock(&Buffer_Manager_latch);
		return FAIL;
	}

	int i;
	pthread_mutex_unlock(&Buffer_Manager_latch);

	for (i = 0; i < Buffer_Manager.cnt; i++)
		close_table(i + 1);

	pthread_mutex_lock(&Buffer_Manager_latch);
	
	Buffer_Manager.fd.clear();
	Buffer_Manager.hash_table.clear();
	Buffer_Manager.pathname.clear();
	Buffer_Manager.cnt=0;

	free(Buffer);
	Buffer=NULL;
	pthread_mutex_unlock(&Buffer_Manager_latch);
	return SUCCESS;
}

int open_table(char *pathname){
	pthread_mutex_lock(&Buffer_Manager_latch);
	int i;

	for(i=0;i<Buffer_Manager.cnt;i++){ 
		int check = strcmp(pathname, Buffer_Manager.pathname[i].c_str()); 

		if (check == 0 && Buffer_Manager.fd[i] > 0) { // 이미 열림
//			printf("Already open %s table\n", pathname);
			pthread_mutex_unlock(&Buffer_Manager_latch);
			return i + 1;
		}
		else if(check == 0){ // 한 번더 열기
//			printf("OPEN SUCCESS 2 \n");
			Buffer_Manager.fd[i]=open_file(pathname);
			pthread_mutex_unlock(&Buffer_Manager_latch);
			return i + 1;
		}
	}


	i = Buffer_Manager.cnt++; // 없으면 추가
	Buffer_Manager.fd.push_back(open_file(pathname));
	Buffer_Manager.pathname.push_back(pathname);
	std::unordered_map<pagenum_t, fnum_t> tmp;
	Buffer_Manager.hash_table.push_back(tmp);
	pthread_mutex_unlock(&Buffer_Manager_latch);
	return i+1;
}

int close_table(int table_id){
	pthread_mutex_lock(&Buffer_Manager_latch);
	int fd = Buffer_Manager.fd[table_id - 1];

	if (fd < 0){
		pthread_mutex_unlock(&Buffer_Manager_latch);
		return FAIL;
	}
	else if( Buffer_Manager.hash_table[table_id - 1].empty()){
		Buffer_Manager.fd[table_id - 1] = -1;
		close_file(fd);
		pthread_mutex_unlock(&Buffer_Manager_latch);
		return SUCCESS;
	}
	
	auto iter = Buffer_Manager.hash_table[table_id - 1].begin();
	int now = iter->second;
	pthread_mutex_lock(&Buffer[now].page_latch);
	pthread_mutex_unlock(&Buffer_Manager_latch);
	buffer_write_frame(table_id, now);
	for(++iter; iter!=Buffer_Manager.hash_table[table_id-1].end();iter++){
		pthread_mutex_lock(&Buffer[iter->second].page_latch);
		pthread_mutex_unlock(&Buffer[now].page_latch);
		now = iter->second;
		buffer_write_frame(table_id, now);
	}
	pthread_mutex_unlock(&Buffer[now].page_latch);

	Buffer_Manager.fd[table_id - 1] = -1;
	Buffer_Manager.hash_table[table_id - 1].clear();
	close_file(fd);

	return SUCCESS;
}

void buffer_write_frame(int table_id, int fnum){
	if (Buffer[fnum].is_dirty == 0 || table_id <= 0) {
		memset(&Buffer[fnum].page, 0, sizeof(page_t));
		return;
	}
		
	// is_dirty	
	if(Buffer[fnum].pnum) // leaf, internal, free
		file_write_page(Buffer_Manager.fd[table_id - 1], Buffer[fnum].pnum, &Buffer[fnum].page);
	else //header 일때
		file_write_header_page(Buffer_Manager.fd[table_id - 1], &Buffer[fnum].hpage);

	memset(&Buffer[fnum].page, 0, sizeof(page_t));
	Buffer[fnum].is_dirty = 0;
}

fnum_t buffer_read_frame(int table_id, pagenum_t pnum){
	fnum_t fnum;
	
	pthread_mutex_lock(&Buffer_Manager_latch);
	
	auto iter = Buffer_Manager.hash_table[table_id-1].find(pnum);
	if (iter != Buffer_Manager.hash_table[table_id-1].end()) { // 해당 페이지가 이미 존재할 때
		fnum = iter->second;
		pthread_mutex_lock(&Buffer[fnum].page_latch);
		LRU_change(fnum);
		pthread_mutex_unlock(&Buffer_Manager_latch);
		return fnum;
	}
	
	// 존재하지 않는다면 	

	fnum = buffer_alloc_frame();
	
	LRU_change(fnum);
	pthread_mutex_unlock(&Buffer_Manager_latch);
	Buffer[fnum].table_id = table_id;
	Buffer[fnum].pnum = pnum;
	Buffer_Manager.hash_table[table_id-1].insert(mp( pnum, fnum ));
	
	if(pnum)
		file_read_page(Buffer_Manager.fd[table_id-1], pnum, &Buffer[fnum].page);
	else
		file_read_header_page(Buffer_Manager.fd[table_id-1], &Buffer[fnum].hpage);
		
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
		Buffer[prev].next=Buffer[now].next;
		Buffer[next].prev=Buffer[now].prev;
	}
	Buffer[tail].next = now;
	Buffer[now].prev =tail;
	Buffer[now].next =0;
	tail= now;
}

fnum_t buffer_alloc_frame(){
	pthread_mutex_lock(&Buffer[0].page_latch);
	fnum_t start = Buffer[0].next;

	if (Buffer[0].now < Buffer[0].size) {// 여유가 있음
		start = ++Buffer[0].now;
		pthread_mutex_lock(&Buffer[start].page_latch);
	}
	else { //LRU Policy
		pthread_mutex_lock(&Buffer[start].page_latch);	
		buffer_write_frame(Buffer[start].table_id, start); //eviction
		Buffer_Manager.hash_table[Buffer[start].table_id -1].erase(Buffer[start].pnum);
	}
	pthread_mutex_unlock(&Buffer[0].page_latch);
	return start;
}

fnum_t buffer_alloc_page(int table_id) {
	fnum_t Header = buffer_read_frame(table_id, 0);
	pthread_mutex_lock(&Buffer_Manager_latch);
	LRU_change(Header);

	fnum_t Free;
	pagenum_t Free_pnum = Buffer[Header].hpage.Free_page;

	if (Free_pnum) {
		Free = buffer_read_frame(table_id, Free_pnum);
		Buffer[Header].hpage.Free_page = Buffer[Free].page.Next_Free;
	}
	else {
		Free = buffer_alloc_frame();
		Free_pnum = Buffer[Header].hpage.Number_of_pages++;
	}
	buffer_unpin(Header);
	LRU_change(Free);
	Buffer_Manager.hash_table[table_id-1].insert(mp(Free_pnum, Free ));
	pthread_mutex_unlock(&Buffer_Manager_latch);	
	Buffer[Free].table_id = table_id;
	Buffer[Free].pnum = Free_pnum;	
	
	return Free;
}


bool is_open(int table_id) {
	if (Buffer_Manager.fd[table_id - 1] < 0 || Buffer_Manager.cnt < table_id)
		return false;

	return true;
}

void buffer_unpin(fnum_t fnum) {
	Buffer[fnum].is_dirty = 1;
	pthread_mutex_unlock(&Buffer[fnum].page_latch);
}

void buffer_unpin_no_dirt(fnum_t fnum) {
	pthread_mutex_unlock(&Buffer[fnum].page_latch);
}



// project5 

bool operator<(const Record &a, my_key_t b){ // binary search for find_leaf
	return a.key < b;
}

bool operator<(my_key_t b, const Index &a){ // binary search for find_leaf
	return b < a.key;
}


void trx_abort(int trx_id) {
	trx_t* now_trx = get_trx(trx_id);
	if(now_trx == NULL)
		return;

	for(int i = now_trx->my_log.size() - 1; i>=0; --i){
		log_t now_log = now_trx->my_log[i];
		fnum_t fnum =buffer_read_frame(now_log.table_id, now_log.pnum);
		int idx = std::lower_bound(Buffer[fnum].page.Records, Buffer[fnum].page.Records+Buffer[fnum].page.Num_Keys, now_log.key)-Buffer[fnum].page.Records;	
		strcpy(Buffer[fnum].page.Records[idx].value, now_log.val);
		buffer_unpin(fnum);
	}
	
    trx_destroy(trx_id);
}

