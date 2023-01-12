#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Fonction pour test le alloc de la mèmoire
int main(int argc, char *argv[]){
    mem_init(get_memory_adr(), get_memory_size());
    
    printf("Test de la fonction mem_alloc\n");
    void *ptr = mem_alloc(100);
    for(int i = 0; i < 100; i++){
        ((char*)ptr)[i] = 'a';
        printf("PASSED\n");
    }


    
    //mem_show();
    mem_free(ptr);
    //mem_show();
    printf("Test de la fonction mem_realloc\n");
    void *ptr2 = mem_alloc(100);
    mem_realloc(ptr2, 200);
    //mem_show();
    mem_free(ptr2);

    return 0;
}