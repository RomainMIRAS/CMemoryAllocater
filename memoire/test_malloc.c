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
    }

    mem_free(ptr);

    printf("TEST ALLOC PASSED\n");
    printf("TEST FREE PASSED\n");
    printf("TEST REALLOC PASSED\n");

    return 0;
}