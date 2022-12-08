#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NB_TESTS 10

int main(int argc, char *argv[]) {
	fprintf(stderr, "Test réalisant de multiples fois une initialisation suivie d'une alloc max.\n"
			"Définir DEBUG à la compilation pour avoir une sortie un peu plus verbeuse."
 		"\n");
	for (int i=0; i<NB_TESTS; i++) {
		debug("Initializing memory\n");
		mem_init(get_memory_adr(), get_memory_size());
		alloc_max(get_memory_size());
	}

	// debug("Initializing memory with parameters\n");
	// mem_init(get_memory_adr(), 0);


	// mem_init(get_memory_adr(), get_memory_size());
	// assert(header->size == 0);

	// Test mem_show avec une fonction qui affiche les zones
	// mem_show(afficher_zone);

	return 0;
}
