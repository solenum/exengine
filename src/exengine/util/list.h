/* list
* A simple arbitrary linked-list implementation.
*/

#ifndef EX_LIST_H
#define EX_LIST_H

#include <stdlib.h>
#include <stdio.h>

typedef struct ex_list_node_t ex_list_node_t;
typedef ex_list_node_t ex_list_t;

struct ex_list_node_t {
  void *data;
  struct ex_list_node_t *next, *prev;
};

/**
 * [ex_list_new initialize a new list]
 * @return [ex_list_t pointer]
 */
static ex_list_node_t* ex_list_new()
{
  ex_list_node_t *n = malloc(sizeof(ex_list_node_t));
  n->prev = NULL;
  n->next = NULL;
  n->data = NULL;

  return n;
}

/**
 * [ex_list_add add a new element to the list]
 * @param n    [ex_list_t to add element]
 * @param data [void pointer of data to add]
 */
static void ex_list_add(ex_list_node_t *n, void *data)
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
  n->next = malloc(sizeof(ex_list_node_t));
  n->next->prev = n;
  n->next->next = NULL;
  n->next->data = NULL;
}

/**
 * [ex_list_remove remove an element from the list]
 * @param  n    [ex_list_t to remove element]
 * @param  data [void pointer of data to remove]
 * @return      [restructured ex_list_t pointer]
 */
static ex_list_t* ex_list_remove(ex_list_node_t *n, void *data)
{
  ex_list_t *root = n;

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
 * [ex_list_destroy cleanup mallocs etc]
 * @param n [ex_list_t pointer to delete]
 */
static void ex_list_destroy(ex_list_node_t *n)
{
  if (n == NULL)
    return;

  // remove first node
  ex_list_node_t *next = n->next;
  free(n);

  // remove the remaining nodes
  while (next != NULL) {
    n = next;
    next = next->next;
    free(n);
  }
}

#endif // EX_LIST_H