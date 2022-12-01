#include "url_include.h"
#pragma once

#define URL_CACHE_TBL_LIMIT         1000000
#define URL_CACHE_TBL_SIZE_THRESH   90
#define URL_CACHE_TBL_DEFAULT_SIZE  1024
#define URL_CACHE_TBL_SCALE_FACTOR  2
#define URL_CACHE_TBL_GC_WALK_LIMIT 2000

#define URL_ENTRY_DIGEST_LEN        16

//extern static int32_t lock;

typedef enum url_cache_ret_ {
    URL_CACHE_SUCCESS = 0,
    URL_CACHE_FAILURE,
    URL_CACHE_ERROR
} url_cache_ret_code_t;

typedef struct url_cache_entry_ {
    uint8_t *url;
    uint32_t ttl;
    uint32_t response_code;
    uint8_t *digest;
    struct url_cache_entry_ *next;
} url_cache_entry_t;

typedef url_cache_entry_t * ucentry;

typedef struct url_cache_table_node_ {
    uint8_t count;
    uint32_t index;
    ucentry next;
} url_cache_table_node_t;

typedef url_cache_table_node_t * uctnode;

typedef struct url_cache_timer_vector_ {
    uint32_t ttl;
    uint32_t index;
} url_cache_timer_vector_t;

typedef url_cache_timer_vector_t * uc_vector;

typedef struct url_cache_table_ {
    uint32_t limit;
    uint32_t size;
    uint32_t total_count;
    uint32_t lookup_success_count;
    uint32_t lookup_failure_count;
    uctnode  table;
    uc_vector gc_vector;
    uint32_t gc_last_index;
    uint32_t gc_count;
    uint32_t gc_cleanup_count;
} url_cache_table_t;

typedef url_cache_table_t * uctable;

uint32_t
url_cache_hash(uint8_t *key, uint32_t len, uint32_t size);

uctable
url_cache_create_table(uint32_t size, uint32_t limit);

uctable
url_cache_resize_table(uctable table);

ucentry
url_cache_get_new_entry(char *url, uint32_t ttl, uint32_t response_code);

url_cache_ret_code_t
url_cache_insert_entry(uctable table, ucentry entry, bool is_update);

url_cache_ret_code_t
url_cache_update_entry(uctable table, ucentry entry);

url_cache_ret_code_t
url_cache_delete_entry(uctable table, ucentry node);

url_cache_ret_code_t
url_cache_delete_entry_at_index(uctable table, uint32_t index, ucentry entry);

ucentry
url_cache_lookup(uctable table, uint8_t *url);

void
url_cache_print_table(uctable table);

void url_cache_table_lock();

void url_cache_table_unlock();

uint32_t url_cache_cleanup(uctable table);

void url_cache_print_table_stats(uctable table);

void url_cache_print_gc_vector(uctable table);
