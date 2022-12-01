#include "url_include.h"

int main(){

    int i, n;
    uctable table = NULL;
    ucentry entry = NULL;
    uint32_t ttl, response_code;
    char url[256] = {0};
    FILE *fp = NULL;


    printf("\nEnter the hash-table size :");
    scanf("%d", &n);
    table = url_cache_create_table(n, URL_CACHE_TBL_LIMIT);
    printf("\nHash table contents are:\n");
    url_cache_print_table(table);

    printf("\nEnter the number of elements to be added to hash-table :");
    scanf("%d", &n);
    fp = fopen("tc.txt", "r");
    if(!fp){
        printf("\nFailed to open file:\n");
        return 0;
    }
    printf("\nEnter %d elements(url, ttl, response_code):\n", n);
    for(i=0;i<n;i++){
        fscanf(fp, "%s %d %d", url, &ttl, &response_code);
        entry = url_cache_get_new_entry(url, ttl, response_code);
        url_cache_insert_entry(table, entry, false);
    }
    fclose(fp);
    printf("\nHash table contents are:\n");
    url_cache_print_table(table);
    url_cache_delete_entry_at_index(table, 127, NULL);
    url_cache_delete_entry_at_index(table, 230, NULL);
    url_cache_delete_entry_at_index(table, 0, NULL);
    //url_cache_print_table_stats(table);
#if 0
    for(;;){
        url_cache_cleanup(table);
        url_cache_print_table_stats(table);
        sleep(1);
    };
#endif
}
