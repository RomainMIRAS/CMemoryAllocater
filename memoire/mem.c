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
struct allocator_header
{
	size_t memory_size;
	mem_fit_function_t *fit;
	struct fb *first_free;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void *memory_addr;

static inline void *get_system_memory_addr()
{
	return memory_addr;
}

static inline struct allocator_header *get_header()
{
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

static inline size_t get_system_memory_size()
{
	return get_header()->memory_size;
}

struct fb
{
	size_t size;
	struct fb *next;
};

struct fb *get_fb_prev(struct fb *fb)
{

	struct fb *current = get_header()->first_free;

	while (current != NULL && current->next != fb)
	{
		current = current->next;
	}

	return current;
}

void mem_init(void *mem, size_t taille)
{
	memory_addr = mem;
	*(size_t *)memory_addr = taille;
	/* On vérifie qu'on a bien enregistré les infos et qu'on
	 * sera capable de les récupérer par la suite
	 */
	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());
	// TODO
	mem_fit(&mem_fit_first);
	get_header()->memory_size = taille;

	struct fb *first_fb;
	first_fb = (struct fb *)(get_header() + 1);
	first_fb->size = get_system_memory_size() - sizeof(struct allocator_header) - sizeof(size_t);
	first_fb->next = NULL;
	get_header()->first_free = first_fb;
}

void mem_show(void (*print)(void *, size_t, int))
{
	/* ... */

	struct fb *Current_free = get_header()->first_free;
	void *Current = get_header() + 1;
	size_t size = 0;

	if (Current_free == NULL) // Si aucun free block
	{
		while (Current < get_system_memory_addr() + get_system_memory_size())
		{
			size = *((size_t *)Current);
			print(Current, size, 0);
			Current = Current + size + sizeof(size_t);
		}
	}
	else
	{								 // Si au moins un free block
		while (Current_free != NULL) // Tant qu'il y a des free blocks
		{
			while (Current != (void *)Current_free) // Tant qu'on est pas sur le free block
			{
				size = *((size_t *)Current);
				print(Current, size, 0);
				Current = Current + size + sizeof(size_t);
			}
			size = *((int *)Current);
			print(Current_free, size, 1);
			Current_free = Current_free->next;
			Current = Current + size + sizeof(size_t);
		}
	}
}

void mem_fit(mem_fit_function_t *f)
{
	get_header()->fit = f;
}

size_t align(size_t taille) {
	return taille%8 == 0 ? taille : 8-(taille%8) + taille;
}


void *mem_alloc(size_t taille)
{
	taille = align(taille);

	struct fb *fb = get_header()->fit(get_header()->first_free, taille);
	if (fb == NULL)
		return NULL; // Si Aucun Free block dispo

	struct fb *fb_prev = get_fb_prev(fb);

	int espaceRestant = fb->size - taille - sizeof(size_t) + 8;

	if (espaceRestant == 0)
	{

		if (fb_prev == NULL)
		{
			get_header()->first_free = fb->next;
		}
		else
		{
			fb_prev->next = fb->next;
		}

		fb->size = taille;
		return ((void *)fb) + sizeof(size_t);
	}
	else
	{
		if (espaceRestant < sizeof(fb) + (int)8) {
			taille += espaceRestant;
		}
		struct fb *new_fb = ((void *)fb) + taille + sizeof(size_t);

		if (fb_prev == NULL)
		{
			get_header()->first_free = new_fb;
		}
		else
		{
			fb_prev->next = new_fb;
		}
		new_fb->size = fb->size - taille - sizeof(size_t);
		new_fb->next = fb->next;

		fb->size = taille;
		return ((void *)fb) + sizeof(size_t);
	}
}

// Actualise la mémoire pour la fusion des zones
void refresh_mem(){
	struct fb *current = get_header()->first_free;
	while(current != NULL){
		// fusion
		if (current->next != NULL && (void *)current + current->size + sizeof(size_t) == (void *)current->next){
			current->size += current->next->size + sizeof(size_t);
			current->next = current->next->next;
		}  else {
			current = current->next;
		}
	}

}

void mem_free(void *mem)
{
	// Taille de la future zone libre
	size_t *taille_zone = (size_t *)(mem - sizeof(size_t));

	struct fb *current = get_header()->first_free;

	// TODO FIX ME
	while (current != NULL && ((void *)current) < mem)
	{
		current = current->next;
	}

	struct fb *prev = get_fb_prev(current);

	struct fb *new_fb = (struct fb *)(mem - sizeof(size_t));

	if (prev == NULL)
	{
		get_header()->first_free = new_fb;
	}
	else
	{
		prev->next = new_fb;
	}

	new_fb->next = current;
	new_fb->size = *taille_zone;

	// Fusion avec le next free block si il est contigu
	if (current != NULL && (void *)current == mem + *taille_zone)
	{
		new_fb->size += current->size + sizeof(size_t);
		new_fb->next = current->next;
	}

	// Fusion avec le previous free block si il est contigu
	if (prev != NULL && (void *)prev + prev->size + sizeof(size_t) == mem)
	{
		prev->size += *taille_zone + sizeof(size_t);
		prev->next = new_fb->next;
	}
	
	// Pour la fusion des zones libres
	refresh_mem();
}

struct fb *mem_fit_first(struct fb *list, size_t size)
{
	struct fb *Current = list;
	while (Current != NULL)
	{
		if (Current->size >= size)
		{
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
size_t mem_get_size(void *zone)
{
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return *((size_t *)zone - sizeof(get_header()->memory_size)); // fépaça attention non accurate
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb *mem_fit_best(struct fb *list, size_t size)
{
	struct fb *Current = list;
	size_t min_size = list->size;
	struct fb * min = list;

	while (Current != NULL)
	{
		if (Current->size < min_size)
		{
			min_size = Current->size;
			min = Current;
		}
		Current = Current->next;
	}
	return min;
}

struct fb *mem_fit_worst(struct fb *list, size_t size)
{
	struct fb *Current = list;
	size_t max_size = list->size;
	struct fb * max_fb = list;

	while (Current != NULL)
	{
		if (Current->size > max_size)
		{
			max_size = Current->size;
			max_fb = Current;
		}
		Current = Current->next;
	}
	return max_fb;

}