#include "url_include.h"



int main(){

    int i, n;
    uctable table = NULL;
    ucentry entry = NULL;
    uint32_t ttl, response_code;
    char url[256] = {0};
    char *lookup_url = "https://en.wikipedia.org/wiki/Main_Page/1";
    FILE *fp = NULL;
    time_t start, end;

    printf("\nEnter the hash-table size :");
    scanf("%d", &n);
    table = url_cache_create_table(n, URL_CACHE_TBL_LIMIT);

    printf("\nEnter the number of elements to be added to hash-table :");
    scanf("%d", &n);
    fp = fopen("test/tc.txt", "r");
    if(!fp){
        printf("\nFailed to open file:\n");
        return 0;
    }
    start = time(NULL);
    printf("\nEnter %d elements(url, ttl, response_code):\n", n);
    for(i=0;i<n;i++){
        fscanf(fp, "%s %d %d", url, &ttl, &response_code);
        entry = url_cache_get_new_entry(url, ttl, response_code);
        url_cache_insert_entry(table, entry, false);
        table = url_cache_resize_table(table);
    }
    end = time(NULL);
    printf("\nElapsed time to scale DB :%lds\n",end-start);
    fclose(fp);


    i = 0;
    printf("Lookup-URL :%s\n", url);
    for(;;){
        url_cache_lookup(table, lookup_url);
        url_cache_cleanup(table);
        url_cache_print_table_stats(table);
        sleep(1);
    };
}
