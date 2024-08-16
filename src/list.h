#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>
#include <stdlib.h>

#define OP_OK 1
#define OP_NOT_OK 0

typedef struct {
  void **arr;
  size_t arr_size;
  size_t occ_size;
} List;

int init_list(List *list);

void deinit_list(List *list, void (*func_p)(void *));

void *get_list(List *list, size_t index);

int append_list(List *list, void *item);

int remove_list(List *list, size_t index, void (*remove_func_p)(void *));

#endif // !LIST_H
