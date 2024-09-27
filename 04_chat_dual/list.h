#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>

/* Data Types */

typedef void (*ConsumerFunction)(void*);

typedef void (*IterationFunction)(void*, void*);

typedef bool (*CompareFunction)(void*, void*);

/* Forward Declaration */
struct list_t;

/* Object Definition */
typedef struct list_t* List;

/* Public Functions */

List list_new(void);

void list_delete(List list, ConsumerFunction deleter);

void *list_push_back(List list, void *object);

void *list_pop_back(List list);

void *list_front(const List list);

void *list_back(const List list);

size_t list_size(const List list);

bool list_empty(const List list);

void *list_remove(List list, size_t index);

void *list_get(const List list, size_t index);

void list_foreach(const List list, IterationFunction function, void *args);

void *list_search(const List list, CompareFunction function, void *object);

bool list_search_remove(const List list, CompareFunction function, void *object, ConsumerFunction deleter);

bool list_sort(List list, CompareFunction function);

#endif
