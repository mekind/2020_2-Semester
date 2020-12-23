#include "buffer.h"
#include "log_manager.h"
#include<stdlib.h>
#include<algorithm>
#include<cstring>
#define SUCCESS 0
#define FAIL 1

extern log_buffer_t LOG_BUFFER;

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
	
	LOG_BUFFER.data = (char *)malloc(sizeof(char) * (10000000));
	memset(LOG_BUFFER.data, 0, sizeof(char) * (10000000));
	LOG_BUFFER.size=0;
	LOG_BUFFER.now_LSN = 0;
	Buffer = (frame_t *)malloc(sizeof(frame_t) * (num_buf + 1));
	memset(Buffer, 0, sizeof(frame_t) * (num_buf + 1));
	Buffer[0].size=num_buf;
	pthread_mutex_unlock(&Buffer_Manager_latch);
	//open_table("DATA00.db");
	//LOG_BUFFER.now_LSN = recovery_log("log_path", "logmsg_path", 0, 0);
	return SUCCESS;
}

int init_db (int buf_num , int flag, int log_num , char* log_path , char* logmsg_path){
/*
• If success, return 0, Otherwise, return a non zero value.
• Do recovery after initialization in this function. (DBMS initialization --> Analysis Redo Undo)
• Log file will be made using log_path
• Log message file will be made using logmsg_path
• flag is needed for recovery test, 0 means normal recovery protocol, 1 means REDO CRASH, 2 means UNDO CRASH.
• log_num is needed for REDO/UNDO CRASH, which means the function must return 0 after the number of logs is processed.
*/
	pthread_mutex_lock(&Buffer_Manager_latch);
	if(Buffer){
		//printf("Already init\n");
		pthread_mutex_unlock(&Buffer_Manager_latch);
		return FAIL;
	}
	
	Buffer = (frame_t *)malloc(sizeof(frame_t) * (buf_num + 1));
	memset(Buffer, 0, sizeof(frame_t) * (buf_num + 1));
	Buffer[0].size=buf_num;

	LOG_BUFFER.data = (char *)malloc(sizeof(char) * (10000000));
	memset(LOG_BUFFER.data, 0, sizeof(char) * (10000000));
	LOG_BUFFER.size=0;
	LOG_BUFFER.now_LSN = 0;
	pthread_mutex_unlock(&Buffer_Manager_latch);

	for(int i=1;i<=10;++i){
		char str[20];
		sprintf(str, "DATA%d", i);
		open_table(str);
	}
	LOG_BUFFER.now_LSN = recovery_log(log_path, logmsg_path, flag, log_num);
	if(flag){
		flush_log(1);
		shutdown_db();
		return FAIL;
	}
	
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

	for (i = 0; i < 10; i++)
		close_table(i + 1);

	pthread_mutex_lock(&Buffer_Manager_latch);
	flush_log(1);
	free(Buffer);
	free(LOG_BUFFER.data);
	Buffer=NULL;
	pthread_mutex_unlock(&Buffer_Manager_latch);
	return SUCCESS;
}

int open_table(char *pathname){
	pthread_mutex_lock(&Buffer_Manager_latch);
	int now;

	if(strlen(pathname) == 5 ) // table id 1~9
		now = pathname[4] - '0';
	else // 10
		now = 10;
	
//	printf("%d now \n",now);
	// 이미 열려있는 경우 
	if(Buffer_Manager.fd[now - 1] > 0) {
		pthread_mutex_unlock(&Buffer_Manager_latch);
		return now;
	}
		

	Buffer_Manager.fd[now - 1]=open_file(pathname);

	pthread_mutex_unlock(&Buffer_Manager_latch);
	return now;
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
	Buffer_Manager.hash_table[table_id-1].insert(mp( pnum, fnum ));
	pthread_mutex_unlock(&Buffer_Manager_latch);
	Buffer[fnum].table_id = table_id;
	Buffer[fnum].pnum = pnum;
	
	
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
		flush_log(1);
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
	if (Buffer_Manager.fd[table_id - 1] < 0 || 10 < table_id || table_id < 1)
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


int trx_abort(int trx_id) {
	trx_t* now_trx = get_trx(trx_id);
	if(now_trx == NULL)
		return 0;

	for(int i = now_trx->my_log.size() - 1; i>=0; --i){
		my_log_t now_log = now_trx->my_log[i];
		fnum_t fnum =buffer_read_frame(now_log.table_id, now_log.pnum);
		int idx = std::lower_bound(Buffer[fnum].page.Records, Buffer[fnum].page.Records+Buffer[fnum].page.Num_Keys, now_log.key)-Buffer[fnum].page.Records;	
		uint64_t now_LSN = make_compensate_log(now_trx->prev_LSN, trx_id, 4, now_log.table_id, now_log.pnum, Buffer[fnum].page.Records[idx].value, now_log.val, 0, idx);
		now_trx->prev_LSN = now_LSN;
		Buffer[fnum].page.pLSN = now_trx->prev_LSN;		
		strcpy(Buffer[fnum].page.Records[idx].value, now_log.val);
		buffer_unpin(fnum);	
	}
	
    trx_destroy(trx_id);
	make_log(now_trx->prev_LSN, trx_id, 3);
	flush_log(1);
	return trx_id;
}

