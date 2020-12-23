#include "log_manager.h"
#include "buffer.h"
#include<string.h>
#include<cstdio>
#include<algorithm>
#include<set>
#include<stdlib.h>
#include<vector>
#define LBUFFER_MAX 10000000

extern frame_t *Buffer;
pthread_mutex_t log_buffer_latch = PTHREAD_MUTEX_INITIALIZER;
log_buffer_t LOG_BUFFER;


int log_fd;
FILE* logms_fd;

void my_print_log(const log_t &a){
    printf("size: %d\n", a.log_size);
    printf("LSN: %d\n", a.LSN);
    printf("pLSN: %d\n", a.prev_LSN);
    printf("trx : %d\n", a.trx_id);
    printf("type: %d\n", a.type);
    if(a.type != 0 && a.type!=2){
        printf("table_id: %d\n", a.table_id);
        printf("pnum: %d\n", a.pnum);
        printf("datalen : %d\n", a.datalen);
    }    
}

uint64_t recovery_log(char* log_path, char* logmsg_path, int flag, int log_num){
    pthread_mutex_lock(&log_buffer_latch);
    log_fd = open(log_path, O_RDWR | O_CREAT, OPEN_MODE);
    logms_fd = fopen(logmsg_path, "w+");

    fprintf (logms_fd , "[ANALYSIS] Analysis pass start\n");
    //printf ("[ANALYSIS] Analysis pass start\n");

    
    std::set<int> winners;
    std::set<int> losers;
    std::vector<log_t> all_log;

    log_t now_log;
    uint64_t now_off = 0;
    uint64_t Last_LSN = 0;
    memset(&now_log, 0, sizeof(log_t));
    uint64_t NextUndo = ULLONG_MAX;
    while(pread(log_fd, &now_log, 28, now_off) == 28){
        if(now_log.log_size == 288){// update, compenstate
            pread(log_fd, &now_log, 288, now_off);
        }
        else if(now_log.log_size == 296){
            pread(log_fd, &now_log, 296, now_off);  
            NextUndo = NextUndo > now_log.next_LSN ? now_log.next_LSN : NextUndo;
        }
        all_log.push_back(now_log);

        if(now_log.type == 0)
            losers.insert(now_log.trx_id);
        else if(now_log.type == 2 || now_log.type == 3){
            losers.erase(now_log.trx_id);
            winners.insert(now_log.trx_id);
        }
        now_off +=now_log.log_size;

        //my_print_log(now_log);
    }
    if(!all_log.empty()) 
        Last_LSN = all_log.back().LSN + all_log.back().log_size;
    fprintf (logms_fd , "[ANALYSIS] Analysis success. Winner: ");
    for(int i: winners)
        fprintf (logms_fd , "%d ",i);
    fprintf (logms_fd , ", LOSER: ");
    for(int i: losers)
        fprintf (logms_fd , "%d ",i);
    fprintf (logms_fd , "\n");
    fprintf(logms_fd , "[REDO] Redo pass start \n");
    int cnt =0 ;

    for(log_t tmp : all_log){
        
        if(tmp.type == 0)
            fprintf(logms_fd , "LSN %lu [BEGIN] Transaction id %d \n", tmp.LSN + tmp.log_size , tmp.trx_id);
        else if(tmp.type == 2)
            fprintf(logms_fd , "LSN %lu [COMMIT] Transaction id %d \n", tmp.LSN + tmp.log_size , tmp.trx_id);
        else if( tmp.type ==3 )
            fprintf(logms_fd , "LSN %lu [ROLLBACK] Transaction id %d \n", tmp.LSN + tmp.log_size , tmp.trx_id);
        else { // redo(update, compenstate)
            fnum_t fnum = buffer_read_frame(tmp.table_id, tmp.pnum);
            if(Buffer[fnum].page.pLSN < tmp.LSN){ // update or compensate
                fprintf(logms_fd , "LSN %lu [UPDATE] Transaction id %d redo apply \n", tmp.LSN + tmp.log_size , tmp.trx_id);
                memcpy(Buffer[fnum].page.Records+tmp.offset, tmp._new, 120);       
                buffer_unpin(fnum);
            }
            else { // consider redo  
                fprintf(logms_fd , "LSN %lu [CONSIDER REDO] Transaction id %d \n", tmp.LSN + tmp.log_size , tmp.trx_id);            
                buffer_unpin_no_dirt(fnum);
            }
                
        }
        if(++cnt == log_num && flag == 1)
            break;
    }

    if(cnt == log_num ){
        fclose(logms_fd);
        pthread_mutex_unlock(&log_buffer_latch);
        return Last_LSN;
    }
    fprintf (logms_fd , "[REDO] Redo pass end\n");
    fprintf (logms_fd , "[UNDO] Undo pass start \n");

    cnt = 0;

     for(int i = all_log.size()- 1 ; i>=0  ; --i){
         log_t tmp = all_log[i];
        if(NextUndo < tmp.LSN) continue;
        if(losers.count(tmp.trx_id) &&(tmp.type == 2 || tmp.type == 4)){
            fnum_t fnum = buffer_read_frame(tmp.table_id, tmp.pnum);
            memcpy(Buffer[fnum].page.Records+tmp.offset, tmp._old, 120);    
            make_compensate_log(tmp.prev_LSN, tmp.trx_id, tmp.type, tmp.table_id, tmp.pnum, tmp._new, tmp._old, tmp.prev_LSN, tmp.offset);
            fprintf(logms_fd , "LSN %lu [CLR REDO] undo lsn %lu \n", tmp.LSN , tmp.prev_LSN); 
            buffer_unpin(fnum);
        }
        if(++cnt == log_num && flag == 2)
            break;
    }

    
    fprintf (logms_fd , "[UNDO] Undo pass end\n");
    fclose(logms_fd);
    //fprintf( fp , "[ANALYSIS] Analysis success. Winner: %d %d .., Loser: %d %d ....\n", winners, losers);
    pthread_mutex_unlock(&log_buffer_latch);
    return Last_LSN;
}

uint64_t make_log(uint64_t prev_LSN, int trx_id, int type){
    pthread_mutex_lock(&log_buffer_latch);
    
    log_t now_log;
    uint64_t ret = LOG_BUFFER.now_LSN;
    now_log.log_size = 28;
    now_log.LSN=LOG_BUFFER.now_LSN;
    now_log.prev_LSN = prev_LSN;
    now_log.trx_id = trx_id;
    now_log.type = type;
    
    memcpy(LOG_BUFFER.data + LOG_BUFFER.size, &now_log, 28);
    /*
    memcpy(&now_log,LOG_BUFFER.data + LOG_BUFFER.size, 296);
    my_print_log(now_log);
    */
    LOG_BUFFER.size += 28;
    LOG_BUFFER.now_LSN+=28;
    //my_print_log(now_log);
    //printf("now buffer size : %d\n",LOG_BUFFER.size);

    if(LOG_BUFFER.size + 296 >= LBUFFER_MAX || type==2 || type ==3)
        flush_log(0);

    pthread_mutex_unlock(&log_buffer_latch);
    return ret;
}
uint64_t make_update_log(uint64_t prev_LSN, int trx_id, int type, int table_id, uint64_t pnum, char* _old, char* _new, int offset){
    pthread_mutex_lock(&log_buffer_latch);
    
    log_t now_log;
    uint64_t ret = LOG_BUFFER.now_LSN;

    now_log.log_size = 288;
    now_log.LSN=LOG_BUFFER.now_LSN;
    now_log.prev_LSN = prev_LSN;
    now_log.trx_id = trx_id;
    now_log.type = 1;
    now_log.table_id= table_id;
    now_log.pnum= pnum;
    now_log.offset = offset;
    now_log.datalen = 120;
    strcpy(now_log._old , _old);
    strcpy(now_log._new , _new);
    
    memcpy(LOG_BUFFER.data + LOG_BUFFER.size, &now_log, 288);
    /*
    memcpy(&now_log,LOG_BUFFER.data + LOG_BUFFER.size, 296);
    my_print_log(now_log);
    */
    LOG_BUFFER.size += 288;
    LOG_BUFFER.now_LSN+=288;
    //my_print_log(now_log);

    if(LOG_BUFFER.size + 296 >= LBUFFER_MAX)
        flush_log(0);

    pthread_mutex_unlock(&log_buffer_latch);
    return ret;
}

uint64_t make_compensate_log(uint64_t prev_LSN, int trx_id, int type, int table_id, uint64_t pnum, char* _old, char* _new, uint64_t next_LSN, int offset){
    pthread_mutex_lock(&log_buffer_latch);
  
    log_t now_log;
    uint64_t ret = LOG_BUFFER.now_LSN;

    now_log.log_size = 296;
    now_log.LSN=LOG_BUFFER.now_LSN;
    now_log.prev_LSN = prev_LSN;
    now_log.trx_id = trx_id;
    now_log.table_id= table_id;
    now_log.type = 4;
    now_log.pnum= pnum;
    now_log.offset = offset;
    now_log.datalen = 120;
    strcpy(now_log._old , _old);
    strcpy(now_log._new , _new);
    now_log.next_LSN = next_LSN;


    memcpy(LOG_BUFFER.data + LOG_BUFFER.size, &now_log, 296);
    /*
    memcpy(&now_log,LOG_BUFFER.data + LOG_BUFFER.size, 296);
    my_print_log(now_log);
    */
    LOG_BUFFER.size += 296;
    LOG_BUFFER.now_LSN+=296;

    
    if(LOG_BUFFER.size + 296 >= LBUFFER_MAX)
        flush_log(0);
    pthread_mutex_unlock(&log_buffer_latch);
    return ret;
}

void flush_log(int flag){
    if(flag)
        pthread_mutex_lock(&log_buffer_latch);
    
    write(log_fd, LOG_BUFFER.data, LOG_BUFFER.size);  
    
    LOG_BUFFER.size=0;
    
    if(flag)
        pthread_mutex_unlock(&log_buffer_latch);
}

