#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__

#include "file.h"
#include <stdint.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h> // max

typedef struct log_t{
    int log_size;
    uint64_t LSN;
    uint64_t prev_LSN;
    int trx_id;
    int type;
    int table_id;
    uint64_t pnum;
    unsigned int offset;
    int datalen;
    char _old[120];
    char _new[120];
    uint64_t next_LSN;
} log_t;

typedef struct log_buffer_t{
    char* data;
    uint64_t now_LSN;
    int size;
} log_buffer_t;


uint64_t recovery_log(char* log_path, char* logmsg_path, int flag, int log_num);
uint64_t make_log(uint64_t prev_LSN, int trx_id, int type);
uint64_t make_update_log(uint64_t prev_LSN, int trx_id, int type, int table_id, uint64_t pnum, char* _old, char* _new, int offset);
uint64_t make_compensate_log(uint64_t prev_LSN, int trx_id, int type, int table_id, uint64_t pnum, char* _old, char* _new, uint64_t next_LSN, int offset);
void flush_log(int flag);
void my_print_log(const log_t &a);

#endif