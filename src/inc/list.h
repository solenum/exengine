#ifndef LIST_H
#define LIST_H

typedef struct list_node_t list_node_t;
typedef list_node_t list_t;

struct list_node_t {
  void *data;
  struct list_node_t *next, *prev;
};

list_node_t* list_new();

void list_add(list_node_t *n, void *data);

list_t* list_remove(list_node_t *n, void *data);

void list_destroy(list_node_t *n);

#endif // LIST_H