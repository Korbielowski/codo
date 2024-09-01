#ifndef ARRAY_H_
#define ARRAY_H_

#include "models.h"

#define OP_OK 1
#define OP_NOT_OK 0

typedef struct {
  void **arr;
  size_t arr_size;
  size_t occ_size;
} Array;

int init_array(Array *array);

void deinit_array(Array *array, void (*func_p)(void *));

void *get_array(Array *array, size_t index);

int append_array(Array *array, void *item);

int remove_array(Array *array, size_t index, void (*remove_func_p)(void *));

void delete_task(Task *task);

void delete_todo(TodoList *todo);

void deinit_task(Task *task);

void deinit_todo(TodoList *todo);

#endif // !ARRAY_H
