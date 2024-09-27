/**
 * Description: A Circular Doubly Linked List, to facilitate
 *              the use of push_back and pop_back operations.
*/

#include "list.h"

#include <stdlib.h>

/* Private Macros */

#define GET_NODE(node, index)\
    for(size_t _c_ = 0; _c_ < index; node = node->next, _c_++)
#define FOREACH(list, iterator)\
    GET_NODE(iterator, list->size)
#define HEAD(list)              ((node_t *) list->contents)
#define TAIL(list)              ((node_t *) HEAD(list)->previous)
#define EMPTY(list)             (list->size == 0)

/* Data Types Implementation */

typedef struct list_t {
    void *contents;
    size_t size;
} list_t;

typedef struct node_t {
    struct node_t *next;
    struct node_t *previous;
    void *content;
} node_t;

/* Private Functions */

static node_t *node_new_head(void *object) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->content = object;

    new_node->next = new_node->previous = new_node;

    return new_node;
}

static node_t *node_new(void *object, node_t *next_node, node_t *previous_node) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->content = object;

    new_node->next = next_node;
    new_node->previous = previous_node;

    next_node->previous = new_node;
    previous_node->next = new_node;

    return new_node;
}

static void node_remove(node_t *node) {
    node_t *next_node = node->next;
    node_t *previous_node = node->previous;

    next_node->previous = previous_node;
    previous_node->next = next_node;
}

static void node_swap(node_t* node01, node_t* node02) {
    /* Swap nodes position */
    node_t temp_node = *node01;
    *node01 = *node02;
    *node02 = temp_node;

    /* Swap next link */
    node_t *temp_next = node01->next;
    node01->next = node02->next;
    node02->next = temp_next;

    /* Swap previous link */
    node_t *temp_prev = node01->previous;
    node01->previous = node02->previous;
    node02->previous = temp_prev;
}

/* Public Functions */

List list_new(void) {
    list_t *list = malloc(sizeof(list_t));
    list->size = 0;
    list->contents = NULL;

    return list;
}

void list_delete(List list, ConsumerFunction deleter) {
    if(!list || EMPTY(list)) {
        return;
    }
    
    node_t *node = HEAD(list);
    size_t i = 0;
    while(i++ < list->size) {
        node_t *next_node = node->next;
        
        if(deleter) {
            deleter(node->content);
        }

        free(node);
        
        node = next_node;
    }

    free(list);
}

void *list_push_back(List list, void *object) {
    if(!list || !object) {
        return NULL;
    }

    if(EMPTY(list)) {
        list->contents = node_new_head(object);
    }
    else {
        node_new(object, HEAD(list), TAIL(list));
    }

    list->size++;

    return object;
}

void *list_pop_back(List list) {
    if(!list || EMPTY(list)) {
        return NULL;
    }

    node_t *last_node = TAIL(list);

    if(list->size == 1) {
        list->contents = NULL;
    }
    else {
        node_remove(last_node);
    }

    void *object = last_node->content;
    
    free(last_node);

    list->size--;

    return object;
}

void *list_front(const List list) {
    if(!list || EMPTY(list)) {
        return NULL;
    }

    return HEAD(list)->content;
}

void *list_back(const List list) {
    if(!list || EMPTY(list)) {
        return NULL;
    }

    return TAIL(list)->content;
}

bool list_empty(const List list) {
    return EMPTY(list);
}

size_t list_size(const List list) {
    return list->size;
}

void *list_remove(List list, size_t index) {
    if(!list || EMPTY(list) || index >= list->size) {
        return NULL;
    }

    node_t *node = HEAD(list);
    if(list->size == 1) {
        list->contents = NULL;
    }
    else {
        GET_NODE(node, index);
        node_remove(node);
    }

    void *object = node->content;
    
    free(node);

    list->size--;

    return object;
}

void *list_get(const List list, size_t index) {
    if(!list || EMPTY(list) || index >= list->size) {
        return NULL;
    }

    node_t *node = HEAD(list);
    GET_NODE(node, index);

    return node->content;
}

void list_foreach(const List list, IterationFunction function, void *args) {
    if(!list || !function || EMPTY(list)) {
        return;
    }

    node_t *node = HEAD(list);
    FOREACH(list, node) {
        function(node->content, args);
    }
}

void *list_search(const List list, CompareFunction function, void *object) {
    if(!list || !function || EMPTY(list) || !object) {
        return NULL;
    }

    node_t *node = HEAD(list);
    FOREACH(list, node) {
        if(function(node->content, object)) {
            return node->content;
        }
    }
    
    return NULL;
}

bool list_search_remove(const List list, CompareFunction function, void *object, ConsumerFunction deleter) {
    if(!list || !function || EMPTY(list) || !object) {
        return false;
    }

    node_t *node = HEAD(list);
    FOREACH(list, node) {
        if(function(node->content, object)) {
            if(list->size == 1) {
                list->contents = NULL;
            }
            else {
                if(node == HEAD(list)) {
                    list->contents = node->next;
                }
                node_remove(node);
            }

            if(deleter) {
                deleter(node->content);
            }

            free(node);

            list->size--;

            return true;
        }
    }
    
    return false;
}

bool list_sort(List list, CompareFunction function) {
    if(!list || !function) {
        return false;
    }

    size_t n = list->size;
    
    for (size_t i = 0; i < n - 1; i++) {
        bool swapped = false;
        
        node_t *node01 = HEAD(list); 
        for (size_t j = 0; j < n - i - 1; j++) {
            node_t *node02 = node01->next;

            if (function(node01->content, node02->content)) {
                node_swap(node01, node02);
                swapped = true;
            }

            node01 = node01->next;
        }

        if (swapped == false) {
            break;
        }
    }

    return true;
}
