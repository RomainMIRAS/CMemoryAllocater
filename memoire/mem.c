/* On inclut l'interface publique */
#include "mem.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* Définition de l'alignement recherché
 * Avec gcc, on peut utiliser __BIGGEST_ALIGNMENT__
 * sinon, on utilise 16 qui conviendra aux plateformes qu'on cible
 */
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/* structure placée au début de la zone de l'allocateur

   Elle contient toutes les variables globales nécessaires au
   fonctionnement de l'allocateur

   Elle peut bien évidemment être complétée
*/
struct allocator_header {
    size_t memory_size;
	mem_fit_function_t *fit;
	struct fb* first_free;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void* memory_addr;

static inline void *get_system_memory_addr() {
	return memory_addr;
}

static inline struct allocator_header *get_header() {
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

static inline size_t get_system_memory_size() {
	return get_header()->memory_size;
}


struct fb {
	size_t size;
	struct fb* next;
	/* ... */
};


void mem_init(void* mem, size_t taille)
{
        memory_addr = mem;
        *(size_t*)memory_addr = taille;
	/* On vérifie qu'on a bien enregistré les infos et qu'on
	 * sera capable de les récupérer par la suite
	 */
	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());
	// TODO 
	mem_fit(&mem_fit_first);
	get_header()->memory_size = taille;

	struct fb * first_fb;
	first_fb = (struct fb *) (get_header()+1);
	first_fb->size = get_system_memory_size()-sizeof(struct allocator_header) - sizeof(size_t) - sizeof(struct fb*);
	first_fb->next = NULL;
	get_header()->first_free = first_fb;
}

void mem_show(void (*print)(void *, size_t, int)) {
	/* ... */

	struct fb* Current_free = get_header()->first_free;
	void* Current = get_header()+1;
	size_t size = 0;	

	while(Current_free != NULL) {
		while(Current != Current_free) {
			size = *((size_t *)Current);
			print(Current,size, 0);
			Current = Current+size+sizeof(size_t);
		}
		size = *((int *)Current);
		print(Current_free, size, 1);
		Current_free = Current_free->next;
	}
}

void mem_fit(mem_fit_function_t *f) {
	get_header()->fit = f;
}

void *mem_alloc(size_t taille) {
	struct fb *fb=get_header()->fit(get_header()->first_free, taille);
	
	return NULL;
}


void mem_free(void* mem) {
}


struct fb* mem_fit_first(struct fb *list, size_t size) {
	struct fb* Current = list;
	while(Current != NULL) {
		if(Current->size >= size) {
			return Current;
		}
		Current = Current->next;
	}
	return NULL;
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return *((size_t *)zone-sizeof(get_header()->memory_size)); //fépaça attention non accurate
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb* mem_fit_best(struct fb *list, size_t size) {
	struct fb* best = list;
	while (list != NULL) {
		list = list->next;
		if (size <= best->size && best->size < list->size) {
			best = list;
		}
	}
	return best->size > size ? best : NULL;
}

struct fb* mem_fit_worst(struct fb *list, size_t size) {
	struct fb* Max = list;
	// Parcours des zones libres
	while(list!=NULL){
		// On récupère celle qui a la taille la plus grande
		if (list->size > Max->size){
			Max = list;
		}
		list = list->next;
	}
	// Si cette taille est inférieure à celle voulu on return NULL
	if (Max->size >= size){
		return NULL;
	}
	return Max;
}
