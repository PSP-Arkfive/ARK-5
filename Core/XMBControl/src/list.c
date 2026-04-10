#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "list.h"

void add_list(List* list, void* item){
    if (list->table == NULL){
        list->table = sce_paf_private_malloc(sizeof(void*)*8);
        sce_paf_private_memset(list->table, 0, sizeof(void*)*8);
        list->max = 8;
        list->count = 0;
    }
    if (list->count >= list->max){
        void** old_table = list->table;
        list->max *= 2;
        list->table = sce_paf_private_malloc(sizeof(void*)*list->max);
        sce_paf_private_memset(list->table, 0, sizeof(void*)*list->max);
        for (int i=0; i<list->count; i++){
            list->table[i] = old_table[i];
        }
        sce_paf_private_free(old_table);
    }
    list->table[list->count++] = item;
}

void clear_list(List* list, void (*cleaner)(void*)){
    if (list->table == NULL) return;
    for (int i=0; i<list->count; i++){
        cleaner(list->table[i]);
    }
    sce_paf_private_free(list->table);
    list->table = NULL;
    list->count = 0;
    list->max = 0;
}