#include "trx_manager.h"
#include "lock_manager.h"

#define mp_record std::unordered_map<int64_t, ptr_lock_table_t>::value_type
#define mp_table std::unordered_map<int, int>::value_type


using um_record =  std::unordered_map<int64_t, ptr_lock_table_t>;
using um_table =  std::unordered_map<int, int>;

std::vector<um_record> v;
um_table tid_to_hash_table; // table_id -> hash_table_idx

pthread_mutex_t lock_table_latch = PTHREAD_MUTEX_INITIALIZER;

extern trx_table_t trx_table;
extern pthread_mutex_t trx_table_latch;

// lock maanger

lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode, uint64_t pnum){
	//printf("acquire1 %d\n",trx_id);
	pthread_mutex_lock(&lock_table_latch); 
	//printf("acquire2 %d\n",trx_id);
	ptr_lock_t ret_lock;
	ptr_lock_table_t now_table = NULL;
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
	
	// ====================== tid에 해당하는 hash table에 record 존재?
	auto iter_table = v[iter_hash->second].find(key);
	
	if(iter_table == v[iter_hash->second].end()){ // key 존재 x
		now_table = make_new_lock_table(table_id, key, iter_hash->second, pnum);	
		v[iter_hash->second].insert(mp_record(key, now_table));
		ret_lock = make_new_waiting(NULL, NULL, now_table, lock_mode, trx_id);
		now_table->head = ret_lock;
		now_table->tail = ret_lock;
		//printf("aaaa\n");
	}
	else if(iter_table->second == NULL){ // key는 존재하나 table이 존재 x
		now_table = make_new_lock_table(table_id, key, iter_hash->second, pnum);	
		iter_table->second = now_table;
		ret_lock = make_new_waiting(NULL, NULL, now_table, lock_mode, trx_id);
		now_table->head = ret_lock;
		now_table->tail = ret_lock;
		//printf("bbb\n");
	}
	else{ //key 존재, table 존재
        now_table = iter_table->second;

        
        lock_t* ttmp = now_table->head;
		int is_exist = 0;
		int mode_sum = 0;
		//이미 획득한 lock이 있는지 확인 
		std::set<int> my_front; ///////////////////////////////////////////////////
		while(ttmp != NULL && !ttmp->is_sleep){
			if(ttmp->trx_id == trx_id ){
				mode_sum |= ttmp->lock_mode; 
				is_exist = 1;
			}
			else 
				my_front.insert(ttmp->trx_id); ///////////////////////////////////////////////////
			ttmp = ttmp->next;
		}
		while(ttmp!=NULL)
		{
			my_front.insert(ttmp->trx_id);
			ttmp = ttmp->next;
		}

		if(mode_sum)
		{ // x mode를 이미 획득한 경우, 항상 가능   ---------------xlock
			// 우선 순위를 위해 앞에다 추가한다.
			ret_lock = make_new_waiting(now_table->head, NULL, now_table, lock_mode, trx_id);  
			now_table->head->prev = ret_lock;
			now_table->head = ret_lock; 
		//	printf("xxx\n");
		}
		else if(is_exist)
		{ // s mode를 이미 획득한 경우
			//printf("already acquire\n");
			if(lock_mode == 0)
			{ // 해당 lock mode도 s인 경우, 항상 가능   ***************************************
				// 우선 순위를 위해 앞에다 추가한다.
				ret_lock = make_new_waiting(now_table->head, NULL, now_table, lock_mode, trx_id);  
				now_table->head->prev = ret_lock;
				now_table->head = ret_lock; 
				//printf("dd\n");
			}
			else 
			{ // 해당 lock이 x mode인 경우, 사이에 아무것도 없어야 가능
				if(now_table->tail->is_sleep || now_table->tail->lock_mode)
				{ // 사이에 다른 trx의 x 존재
					pthread_mutex_unlock(&lock_table_latch);
					//printf("errorr 1\n");
					return NULL; // abort
				}
				else if (deadlock_detection(trx_id, now_table, lock_mode)) 
				{ // 앞에 trx들 중 dead lock 존재?
						//pthread_mutex_unlock(&lock_table_latch);
						//printf("errorr 2\n");
						return NULL;
				}
				else
				{ // sssssx 가능!
					ret_lock = make_new_waiting(NULL, now_table->tail, now_table, lock_mode, trx_id);  
					now_table->tail->next = ret_lock;
		    		now_table->tail = ret_lock; 

					//다른 trx의 s모드가 모두 종료될 때 까지 sleep
						

					int other_trx = 1;
					//printf("sleep1\n");

					while(1)
					{
						lock_t* scan_lock = now_table->head;
						other_trx = 0;
						while(scan_lock != ret_lock)
						{
							if(scan_lock->trx_id != ret_lock->trx_id )
								other_trx++;				
							scan_lock = scan_lock->next;
						}
						if(!other_trx) 
							break;
						
						pthread_cond_wait(&(ret_lock->waiting_cond), &lock_table_latch);
					}
						
				}
			}
		}
		else
		{// 획득한 적이 없는 경우 
			if(deadlock_detection(trx_id, now_table, lock_mode)) // deadlock 검사
			{
				//printf("dead %d\n", trx_id);
				// deadlock이 발견된 경우 atomic하게 처리해야되서 unlock 안함 
				return NULL; // abort
			}
			//printf("eee\n");
			ret_lock = make_new_waiting(NULL, now_table->tail, now_table, lock_mode, trx_id);  
			now_table->tail->next = ret_lock;
		    now_table->tail = ret_lock; 
			ret_lock->is_sleep=1;

			if(ret_lock->lock_mode == 0) // 해당 lock이 share
			{
				// 자기 차례일 때까지 wait (제일 앞에 or 내 앞이 s고 깨어있음)
				while(now_table->head != ret_lock && (ret_lock->prev->is_sleep==1 || ret_lock->prev->lock_mode==1))
					pthread_cond_wait(&(ret_lock->waiting_cond), &lock_table_latch);					
			}
			else // 해당 lock이 x
			{
				// 자기 차례일 때까지 wait
				while(now_table->head != ret_lock)  
					pthread_cond_wait(&(ret_lock->waiting_cond), &lock_table_latch);
			}
			ret_lock->is_sleep=0;
			// shared 일 때 다음 share도 깨우자 
			if(ret_lock->next != NULL && ret_lock->next->lock_mode == 0 && ret_lock->lock_mode==0)
					pthread_cond_signal(&(ret_lock->next->waiting_cond));
		}
	}
	// 획득 성공
	//printf("fin ac\n");
	pthread_mutex_unlock(&lock_table_latch);

	return ret_lock;
}

int lock_release(int trx_id, int is_abort)
{
	pthread_mutex_lock(&trx_table_latch);
    auto iter = trx_table.trx_list.find(trx_id);
    if(iter == trx_table.trx_list.end()) // 이미 abort해서 없음
    {
        pthread_mutex_unlock(&trx_table_latch);
        return trx_id;
    }
    trx_t* now_trx = iter->second;
    pthread_mutex_unlock(&trx_table_latch);
	
	if(!is_abort)
		pthread_mutex_lock(&lock_table_latch);

	std::set<lock_table_t*> will_release;
	ptr_lock_t now_lock = now_trx->head;

	// lock 삭제하기 
	while(now_lock != NULL)
	{
		ptr_lock_table_t now_table = now_lock->sentinel;

		// waiting 순서 변경
    	if( now_table->head == now_lock)
		{  // head 일때
    	    now_table->head = now_lock->next; 
			if(now_lock->next != NULL)
				now_lock->next->prev = NULL;
			else 
				now_table->tail = NULL;
    	} 
    	else if(now_lock->next != NULL) 
		{ //중간에 끼어 있을 때
    	    now_lock->prev->next = now_lock->next;
    	    now_lock->next->prev = now_lock->prev;
    	}
    	else
		{ //tail 일때 
			now_table->tail = now_lock->prev;
    	    now_lock->prev->next = NULL;
    	}
	
		will_release.insert(now_table);
		ptr_lock_t ttmp = now_lock;
		now_lock = now_lock->next_trx;
		free(ttmp);
		ttmp=NULL;
	}

	// 다음 lock 깨우기 
	for(auto iter = will_release.begin(); iter!= will_release.end(); ++iter )
	{
		ptr_lock_table_t now_table = *(iter);
		ptr_lock_t now_lock;
		if(now_table->head == NULL){ // table 에 key 가 없으면 삭제
			v[now_table->idx].erase(now_table->key);
			free(now_table);
			now_table=NULL;
		}	
		else
		{// 있으면 자고 있는 거 하나 깨우기
			now_lock = now_table->head;

			while(now_lock != NULL && now_lock->is_sleep == 0)
				now_lock = now_lock->next;

			if(now_lock != NULL)
				pthread_cond_signal(&(now_lock->waiting_cond));
		}
	}

	pthread_mutex_unlock(&lock_table_latch);
	return 0;
}

int deadlock_detection(int trx_id, ptr_lock_table_t now_table, int lock_mode){
    pthread_mutex_lock(&trx_table_latch);
    trx_t* now_trx = trx_table.trx_list.find(trx_id)->second;
	
    //printf("start deadlock detect\n");
    ptr_lock_t tmp; 

    if(lock_mode) // x mode
    {
        tmp = now_table->head;
        while(tmp!=NULL)
        {
            now_trx->wait_for.insert(tmp->trx_id);
            tmp = tmp->next;
        }
    }
    else
    {   //shared는 연속 되는 s는 제거
        tmp = now_table->tail;
        while(tmp!=NULL && tmp->lock_mode == 0 ) // confilct이 아닌 lock 선별
            tmp = tmp->prev;
        while(tmp!=NULL)
        {
            now_trx->wait_for.insert(tmp->trx_id);
            tmp = tmp->prev;
        }
    }
    
    now_trx->wait_for.erase(trx_id); // 자기 자신은 제외
    std::set<int> visit;

    for(int i : now_trx->wait_for) {
		if(dfs(i, trx_id, visit)){
			pthread_mutex_unlock(&trx_table_latch);
			return 1;
		}
	}
	
	pthread_mutex_unlock(&trx_table_latch);
    return 0;
}

int dfs(int now, int target, std::set<int> &visit) {
	//cycle 발생
	if(now == target)
		return 1;
	else if(visit.count(now))
		return 0;
	visit.insert(now);
	
    auto iter = trx_table.trx_list.find(now);
    // abort 되거나 commit 된 trx
	if(iter == trx_table.trx_list.end())
		return 0;

	trx_t* now_trx = iter->second;

	for(int i : now_trx->wait_for)
		if(dfs(i, target, visit))
			return 1;

	return 0;
}

ptr_lock_t make_new_waiting(ptr_lock_t next, ptr_lock_t prev, ptr_lock_table_t senti, int lock_mode, int trx_id){
	ptr_lock_t new_waiting = (ptr_lock_t)malloc(sizeof(lock_t));

	new_waiting->next =next;
	new_waiting->prev =prev;
	new_waiting->sentinel=senti;
	new_waiting->waiting_cond = PTHREAD_COND_INITIALIZER;
	new_waiting->next_trx= NULL;
	new_waiting->prev_trx= NULL;
	new_waiting->lock_mode=lock_mode;
	new_waiting->is_sleep=0;
    new_waiting->trx_id=trx_id;
	
	return new_waiting;
}

ptr_lock_table_t make_new_lock_table(int table_id, int64_t key, int idx, uint64_t pnum){
	ptr_lock_table_t new_lock_table = (ptr_lock_table_t)malloc(sizeof(lock_table_t));

	new_lock_table->table_id=table_id;
	new_lock_table->key = key;
	new_lock_table->idx =idx;
	new_lock_table->head =NULL;
	new_lock_table->tail =NULL;
    new_lock_table->pnum = pnum;
	return new_lock_table;
}

