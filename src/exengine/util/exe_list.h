/**
* exe_list.h
* A simple arbitrary linked-list implementation.
*/

#ifndef EXE_LIST_H
#define EXE_LIST_H

#include <stdlib.h>
#include <stdio.h>

typedef struct list_node_t list_node_t;
typedef list_node_t list_t;

struct list_node_t {
  void *data;
  struct list_node_t *next, *prev;
};

/**
 * [list_new initialize a new list]
 * @return [list_t pointer]
 */
static list_node_t* list_new()
{
  list_node_t *n = malloc(sizeof(list_node_t));
  n->prev = NULL;
  n->next = NULL;
  n->data = NULL;

  return n;
}

/**
 * [list_add add a new element to the list]
 * @param n    [list_t to add element]
 * @param data [void pointer of data to add]
 */
static void list_add(list_node_t *n, void *data)
{
  // get next empty node
  while (n->data != NULL) {
    if (n->next != NULL)
      n = n->next;
    else
      return;
  }

  // set data and add new node
  n->data = data;
  n->next = malloc(sizeof(list_node_t));
  n->next->prev = n;
  n->next->next = NULL;
  n->next->data = NULL;
}

/**
 * [list_remove remove an element from the list]
 * @param  n    [list_t to remove element]
 * @param  data [void pointer of data to remove]
 * @return      [restructured list_t pointer]
 */
static list_t* list_remove(list_node_t *n, void *data)
{
  list_t *root = n;

  // find node with matching data addr
  while (n->data != data) {
    if (n->next != NULL)
      n = n->next;
    else
      return root;
  }

  // remove node and adjust root
  if (n->prev != NULL) {
    if (n->next != NULL) {
      n->prev->next = n->next;
      n->next->prev = n->prev;
    } else {
      n->prev->next = NULL;
    }
  } else if (n->next != NULL) {
    // reset root node
    root = n->next;
    root->prev = NULL;
  }

  // free node
  n->data = NULL;
  free(n);

  return root;
}

/**
 * [list_destroy cleanup mallocs etc]
 * @param n [list_t pointer to delete]
 */
static void list_destroy(list_node_t *n)
{
  if (n == NULL)
    return;

  // remove first node
  list_node_t *next = n->next;
  free(n);

  // remove the remaining nodes
  while (next != NULL) {
    n = next;
    next = next->next;
    free(n);
  }
}

#endif // EXE_LIST_H