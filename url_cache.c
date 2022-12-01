#include "url_include.h"

static int32_t lock = 1;

void
url_cache_table_lock() {
    while(lock == 0);
    lock -=1;
}

void
url_cache_table_unlock() {
    lock +=1;
}

void
url_cache_decode_entry(ucentry entry, char *decode){

    if(!entry){
        sprintf(decode, "NULL");
        return;
    }
    sprintf(decode, ":%d:", entry->ttl);
    sprintf(decode, "[:]");
    return;
}

uint32_t
url_cache_hash(uint8_t *key, uint32_t len, uint32_t size) {

    int i;
    uint64_t hash = 7;
    for(i=0;i<len; i++){
        hash = hash*19+(uint64_t)key[i];
    }
    return hash%size;
}
#if 0
void
url_cache_get_md5_digest(char *url, uint8_t *buffer){

    uint32_t len;
    MD5_CTX ctx;
    MD5_Init(&ctx);
    len = strlen(url);

    while (len > 0) {
        if (len > 512) {
            MD5_Update(&ctx, url, 512);
        } else {
            MD5_Update(&ctx, url, len);
        }
        len -= 512;
        url += 512;
    }

    MD5_Final(buffer, &ctx);
    return;
}
#endif
int32_t
url_cache_compare_entry(ucentry e1, ucentry e2){

    return strcmp((char *)e1->url, (char *)e2->url);
}

ucentry
url_cache_get_new_entry(char *url, uint32_t ttl, uint32_t response_code){
    ucentry entry = NULL;
    uint32_t len;
    entry = (ucentry)calloc(1, sizeof(url_cache_entry_t));
    if(NULL == entry){
        return NULL;
    }
    len = strlen((char *)url);
    entry->url = (uint8_t *)calloc(len+1, sizeof(uint8_t));
    memcpy(entry->url, url, len);
    entry->ttl = time(NULL)+ttl;
    entry->response_code = response_code;
#if 0
    entry->digest = (uint8_t *)calloc(URL_ENTRY_DIGEST_LEN, sizeof(uint8_t));
    url_cache_get_md5_digest(url, entry->digest);
#endif
    return entry;
}

void *
url_cache_free_entry(ucentry entry){
    if(NULL == entry){
        return NULL;
    }
    /*Free inside allocations*/
    free(entry->url);
    free(entry->digest);
    free(entry);
    entry = NULL;
    return NULL;
}

uctable
url_cache_create_table(uint32_t size, uint32_t limit) {

    uctable table = NULL;
    uctnode tnode = NULL;
    uc_vector  gc_vector = NULL;
    int i = 0;

    if(size > limit){
        printf("\nHash Table creation failed: Size is greater than limit\n");
        return NULL;
    }
    table = (uctable) calloc(1, sizeof(url_cache_table_t));
    if (!table) {
        printf("\nHash Table creation failed\n");
        return NULL;
    }
    table->size = size;
    table->limit = limit;
    tnode = (uctnode) calloc(size, sizeof(url_cache_table_node_t));
    if(!tnode) {
        printf("\nHash Table creation failed\n");
        free(table);
        return NULL;
    }
    table->table = tnode;

    for(i=0;i<size;i++){
        tnode[i].index = i;
        tnode[i].count = 0;
        tnode[i].next = NULL;
    }

    gc_vector = (uc_vector) calloc(size, sizeof(url_cache_timer_vector_t));
    if(!gc_vector) {
        printf("\nHash Table: GC vector creation failed\n");
    }
    table->gc_vector = gc_vector;
    table->gc_last_index = 0;
    table->gc_cleanup_count = 0;

    return table;
}

url_cache_ret_code_t
url_cache_insert_entry(uctable table, ucentry entry, bool is_update) {

    int32_t index;
    uint32_t key_len;
    ucentry tmp = NULL;
    if (!entry || !table || !table->table){
        printf("\nNULL check failed during insertion\n");
        return URL_CACHE_ERROR;
    }

    key_len = strlen((char *)entry->url); 
    index = url_cache_hash(entry->url, key_len, table->size);

    if(index < 0){
        printf("\nHashing failed\n");
        return URL_CACHE_ERROR;
    }

    if(true == is_update && NULL == table->table[index].next) {
        printf("\nUpdate failed\n");
        return URL_CACHE_ERROR;

    }
    /*Handle update code*/

    if(!table->table[index].next){
        url_cache_table_lock();
        table->table[index].next = entry;
        url_cache_table_unlock();
    }
    else {
        tmp = table->table[index].next;
        while(NULL != tmp->next){
            tmp = tmp->next;
        }
        url_cache_table_lock();
        tmp->next = entry;
        url_cache_table_unlock();
    }
    table->table[index].count++;
    table->total_count++;

    /*Add the entry to GC vector*/
    url_cache_table_lock();
    table->gc_vector[(table->gc_last_index)%(table->size)].ttl = entry->ttl;
    table->gc_vector[(table->gc_last_index)%(table->size)].index = index;
    url_cache_table_unlock();

   return URL_CACHE_SUCCESS;
}

url_cache_ret_code_t
url_cache_delete_entry_at_index(uctable table, uint32_t index, ucentry entry) {

    bool        found = false;
    ucentry     tmp = NULL,
                prev = NULL;

    if (!table || !table->table){
        printf("\nNULL check failed during insertion\n");
        return URL_CACHE_ERROR;
    }

    if(index < 0){
        printf("\nHashing failed\n");
        return URL_CACHE_ERROR;
    }

    if((NULL == table->table[index].next)) {
        printf("\nDelete failed\n");
        return URL_CACHE_ERROR;
    }

    tmp = table->table[index].next;
    prev = tmp;
    if (entry) {
        while(NULL != tmp){
            /*Add compare function here*/
            if(0 == url_cache_compare_entry(tmp, entry)) {
                found = true;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        if(found) {
            prev->next = tmp->next;
            
            url_cache_table_lock();
            url_cache_free_entry(tmp);
            table->table[index].count--;
            table->total_count--;
            url_cache_table_unlock();

        } else{
            return URL_CACHE_FAILURE;
        }
    } else{
        url_cache_table_lock();
        while(NULL != tmp){
            /*Delete all nodes at index*/
            url_cache_free_entry(prev);
            tmp = tmp->next;
            prev = tmp;
            table->table[index].count--;
            table->total_count--;
            url_cache_table_unlock();
        }
        table->table[index].next = NULL;
    }
   return URL_CACHE_SUCCESS;
}

url_cache_ret_code_t
url_cache_delete_entry(uctable table, ucentry entry) {

    int32_t     index;
    uint32_t    key_len;

    if (!entry || !table || !table->table){
        printf("\nNULL check failed during insertion\n");
        return URL_CACHE_ERROR;
    }

    key_len = strlen((char *)entry->url); 
    index = url_cache_hash(entry->url, key_len, table->size);

    return url_cache_delete_entry_at_index(table, index, entry);
}

ucentry
url_cache_lookup(uctable table, uint8_t *url) {
    
    int32_t     index;
    uint32_t    key_len;
    bool        found = false;
    ucentry     tmp = NULL;

    if (!url || !table || !table->table){
        printf("\nNULL check failed during lookup\n");
        return NULL;
    }

    key_len = strlen((char *)url); 
    index = url_cache_hash(url, key_len, table->size);

    if(NULL == table->table[index].next) {
        printf("\nLookup failed\n");
        return NULL;
    }

    tmp = table->table[index].next;

    while(NULL != tmp->next){
        /*Add compare function here*/
        if(0 == strcmp((char *)tmp->url, (char *)url)) {
            found = true;
            break;
        }
        tmp = tmp->next;
    }
    if(found){
        return tmp;
    }
    return NULL;
}

void
url_cache_print_table(uctable table){

    uctnode tnode = NULL;
    ucentry entry = NULL;
    int i,j=0;
    char decode[256] = {0};
    if(!table){
        printf("\nTable doesn't exist\n");
        return;
    }

    tnode = table->table;

    for(i = 0; i<table->size;i++){
       printf("\n%4d|",i);
       entry = tnode[i].next;
       j = tnode[i].count;
       while(entry){
           url_cache_decode_entry(entry, decode);
           printf("->[%d]",j);
           entry = entry->next;
           j--;
       }
       printf("->NULL");
    }
    return;
}

void
url_cache_print_table_stats(uctable table){

    if(!table){
        printf("\nTable doesn't exist\n");
        return;
    }
    printf("\nTable total-count :%d\n", table->total_count);
    printf("Table GC clean-count :%d\n", table->gc_cleanup_count);
    printf("+++++++++++++++++++++++++++++++++++\n");
    return;
}

uint32_t
url_cache_cleanup(uctable table) {

    uint32_t cur_time;
    uint32_t end_idx, cur_idx;
    url_cache_ret_code_t ret_code;
    uc_vector gc_vector = NULL;
    gc_vector = table->gc_vector;

    /*cleanup if ttl expires*/
    /*avoiding tight loop, expiring 2K entries per loop*/
    end_idx = table->gc_last_index+URL_CACHE_TBL_GC_WALK_LIMIT;
    while(table->gc_last_index <= end_idx) {
  
        /*Deletes all the entries at that index, this might cause, un-expired
         * entries delete as well*/
        cur_time = time(NULL);
        /*check for timer-expiry*/
        cur_idx = (table->gc_last_index)%table->size;
        if(gc_vector[cur_idx].ttl && (gc_vector[cur_idx].ttl <= cur_time)) {
            //ret_code = url_cache_delete_entry_at_index(table, gc_vector[cur_idx].index, NULL);
            ret_code = URL_CACHE_SUCCESS;
            if(URL_CACHE_SUCCESS == ret_code) {
                /*set the value to zero, as we cleaned up the cache entry*/
                //memset(&gc_vector[cur_idx],0, sizeof(url_cache_timer_vector_t));
                //gc_vector[cur_idx].ttl = 0;
                table->gc_cleanup_count++;
            }
        }
        table->gc_last_index++;
    }
    return table->gc_cleanup_count;
}
