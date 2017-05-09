#include <list.h>
#include <stdlib.h>
#include <stdio.h>

list_node_t* list_new()
{
  list_node_t *n = malloc(sizeof(list_node_t));
  n->prev = NULL;
  n->next = NULL;
  n->data = NULL;

  return n;
}

void list_add(list_t *n, void *data)
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

list_t* list_remove(list_node_t *n, void *data)
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
    if (n->next != NULL)
      n->prev->next = n->next;
    else
      n->prev->next = NULL;
  } else if (n->next != NULL) {
    // reset root node
    root = n->next;
    root->prev = NULL;
  }

  // free node
  free(n);

  return root;
}

void list_destroy(list_node_t *n)
{
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