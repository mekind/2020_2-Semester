#include "trx_manager.h"
#include<set>
#include<stdio.h>
#define mp_trx std::unordered_map<ull, trx_t*>::value_type


trx_table_t trx_table;
pthread_mutex_t trx_table_latch = PTHREAD_MUTEX_INITIALIZER;
extern pthread_mutex_t lock_table_latch;

//transaction manager

int trx_begin(void)
{
    pthread_mutex_lock(&trx_table_latch);
    int trx_id = ++trx_table.cnt;
    trx_t* new_trx = (trx_t*)malloc(sizeof(trx_t));
    if(new_trx==NULL)
    {
       // printf("No more space for transaction\n");
        pthread_mutex_unlock(&trx_table_latch);
        return 0;
    }
    
    trx_table.trx_list.insert(mp_trx( trx_id , new_trx));
    new_trx->head=new_trx->tail= NULL;
    new_trx->wait_for.clear();
    new_trx->my_log.clear();
    //printf("trx begin  %d\n ",trx_table.cnt);
    pthread_mutex_unlock(&trx_table_latch);    
    return trx_id;
}

int trx_commit(int trx_id)
{
	lock_release(trx_id, 0);
    
    pthread_mutex_lock(&trx_table_latch);
    auto iter = trx_table.trx_list.find(trx_id);
    if(iter == trx_table.trx_list.end()) // 이미 abort해서 없음
    {
        pthread_mutex_unlock(&trx_table_latch);
        return trx_id;
    }
    trx_t* now_trx = iter->second;
    now_trx->my_log.clear();
    now_trx->wait_for.clear();
    trx_table.trx_list.erase(trx_id);    
    pthread_mutex_unlock(&trx_table_latch);

    free(now_trx);
    now_trx=NULL;
	
    return trx_id;
}

int make_log_in_trx(int trx_id, const char* val, lock_t* obj) {
    pthread_mutex_lock(&trx_table_latch);
    //printf("start make_log\n");
    auto iter = trx_table.trx_list.find(trx_id);
    if(obj==NULL || iter == trx_table.trx_list.end()) 
    {
        //deadlock detected
        //printf("dead\n");
        pthread_mutex_unlock(&trx_table_latch);
        return 1;
    }

    trx_t* now_trx = iter->second;
    pthread_mutex_unlock(&trx_table_latch);

    if(now_trx->head == NULL)
    {
        now_trx->head=obj;
        now_trx->tail=obj;
    }
    else
    {
        obj->next_trx = NULL;
        obj->prev_trx = now_trx->tail;
        now_trx->tail->next_trx = obj;
        now_trx->tail = obj;
    }
    
    if(obj->lock_mode){
        log_t new_log;
        new_log.pnum = obj->sentinel->pnum;
        new_log.table_id = obj->sentinel->table_id;
        new_log.key = obj->sentinel->key;
        strcpy(new_log.val, val);
        now_trx->my_log.push_back(new_log);
    }
    //printf("end make_log\n");
    
    return 0;
}


trx_t* get_trx(int trx_id){
    pthread_mutex_lock(&trx_table_latch);
    auto iter = trx_table.trx_list.find(trx_id);

    if(iter == trx_table.trx_list.end()){
        pthread_mutex_unlock(&trx_table_latch);
        return NULL;
    }
        

    pthread_mutex_unlock(&trx_table_latch);
    return iter->second;
}



int trx_destroy(int trx_id) // 이미 mutex를 잡은 상태라 commit 과 다름
{    
    lock_release(trx_id, 1);
    
    pthread_mutex_lock(&trx_table_latch);
    auto iter = trx_table.trx_list.find(trx_id);
    if(iter == trx_table.trx_list.end()) // 이미 abort해서 없음
    {
        pthread_mutex_unlock(&trx_table_latch);
        return trx_id;
    }
    trx_t* now_trx = iter->second;
    now_trx->wait_for.clear();
    now_trx->my_log.clear();
    trx_table.trx_list.erase(trx_id);
    pthread_mutex_unlock(&trx_table_latch);

    free(now_trx);
    now_trx=NULL;

    return trx_id;
}