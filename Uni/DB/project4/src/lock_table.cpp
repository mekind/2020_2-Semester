#include <lock_table.h>
#define mp_record std::unordered_map<int64_t, ptr_lock_table_t>::value_type
#define mp_table std::unordered_map<int, int>::value_type
#define now_hash_table v[iter_hash->second]

using um_record =  std::unordered_map<int64_t, ptr_lock_table_t>;
using um_table =  std::unordered_map<int, int>;

pthread_mutex_t lock_table_latch;
std::vector<um_record> v;
um_table tid_to_hash_table; // table_id -> hash_table_idx

int init_lock_table()
{
	/* DO IMPLEMENT YOUR ART !!!!! */
	lock_table_latch = PTHREAD_MUTEX_INITIALIZER;
	return 0;
}

lock_t* lock_acquire(int table_id, int64_t key)
{
	/* ENJOY CODING !!!! */
	pthread_mutex_lock(&lock_table_latch);
	
	ptr_lock_t ret_lock;
	ptr_lock_table_t now_table;
	auto iter_hash = tid_to_hash_table.find(table_id);
	
	if(iter_hash == tid_to_hash_table.end()) // table이 쓰인 적 없음
	{
		// 새로운 hash 테이블 추가
		um_record new_hash_table;
		new_hash_table.insert(mp_record(key, NULL));
		tid_to_hash_table.insert(mp_table(table_id, v.size()));
		v.push_back(new_hash_table);
		iter_hash = tid_to_hash_table.find(table_id);
	}
	
	// ====================== hash table에 record 존재?
	auto iter_table = now_hash_table.find(key);
	
	if(iter_table == now_hash_table.end()){ // key 존재 x
		now_table = make_new_lock_table(table_id, key, iter_hash->second);	
		now_hash_table.insert(mp_record(key, now_table));
		ret_lock = make_new_waiting(NULL, NULL, now_table);
		now_table->head = ret_lock;
		now_table->tail = ret_lock;
	}
	else if(iter_table->second == NULL){ // key는 존재하나 table이 존재 x
		now_table = make_new_lock_table(table_id, key, iter_hash->second);	
		iter_table->second = now_table;
		ret_lock = make_new_waiting(NULL, NULL, now_table);
		now_table->head = ret_lock;
		now_table->tail = ret_lock;
		
	}
	else{ //key 존재, table 존재
		now_table = iter_table->second;
		ret_lock = make_new_waiting(NULL, now_table->tail, now_table);
		now_table->tail->next = ret_lock;
		now_table->tail = ret_lock;

		while(now_table->head != ret_lock) // 자기 차례일 때까지 wait
			pthread_cond_wait(&(ret_lock->waiting_cond), &lock_table_latch);
	}

	pthread_mutex_unlock(&lock_table_latch);

	return ret_lock;
}

int lock_release(lock_t* lock_obj)
{
	/* GOOD LUCK !!! */
	pthread_mutex_lock(&lock_table_latch);

	ptr_lock_table_t now_table = lock_obj->sentinel;

	now_table->head = lock_obj->next; // waiting 순서 변경
	free(lock_obj);
	lock_obj=NULL;
	
	if(now_table->head == NULL){
		v[now_table->idx].erase(now_table->key);
		free(now_table);
		now_table=NULL;
	}		
	else
		pthread_cond_signal(&(now_table->head->waiting_cond));

	pthread_mutex_unlock(&lock_table_latch);
	return 0;
}


ptr_lock_t make_new_waiting(ptr_lock_t next, ptr_lock_t prev, ptr_lock_table_t senti ){
	ptr_lock_t new_waiting = (ptr_lock_t)malloc(sizeof(lock_t));

	new_waiting->next =next;
	new_waiting->prev =prev;
	new_waiting->sentinel=senti;
	new_waiting->waiting_cond = PTHREAD_COND_INITIALIZER;

	return new_waiting;
}

ptr_lock_table_t make_new_lock_table(int table_id, int64_t key, int idx){
	ptr_lock_table_t new_lock_table = (ptr_lock_table_t)malloc(sizeof(lock_table_t));

	new_lock_table->table_id=table_id;
	new_lock_table->key = key;
	new_lock_table->idx =idx;
	new_lock_table->head =NULL;
	new_lock_table->tail =NULL;

	return new_lock_table;
}

