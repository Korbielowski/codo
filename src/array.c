#include "../include/array.h"
#include <stdio.h>
#include <stdlib.h>

int init_array(Array *array) {
  array->arr_size = 10;
  array->occ_size = 0;
  array->arr = malloc(array->arr_size * sizeof(void *));
  if (array->arr == NULL) {
    return OP_NOT_OK;
  }
  return OP_OK;
}

void deinit_array(Array *array, void (*deinit_func_p)(void *)) {
  if (deinit_func_p != NULL) {
    for (int i = 0; i < array->occ_size; i++) {
      printf("destroying index:%d\n", i);
      (*deinit_func_p)(array->arr[i]);
    }
  }
  free(array->arr);
}

void *get_array(Array *array, size_t index) {
  if (index >= array->occ_size) {
    return NULL;
  }
  return array->arr[index];
}

int append_array(Array *array, void *item) {
  array->occ_size++;
  if (array->occ_size > array->arr_size) {
    array->arr_size *= 2;
    array->arr = realloc(array->arr, array->arr_size * sizeof(void *));
    if (array->arr == NULL) {
      return OP_NOT_OK;
    }
  }
  array->arr[array->occ_size - 1] = item;
  return OP_OK;
}

int remove_array(Array *array, size_t index, void (*remove_func_p)(void *)) {
  (*remove_func_p)(array->arr[index]);
  if (index >= array->occ_size) {
    return OP_NOT_OK;
  }
  for (int i = index; i < array->occ_size - 1; i++) {
    void *element = get_array(array, i + 1);
    if (element == NULL) {
      return OP_NOT_OK;
    }
    array->arr[i] = element;
  }
  array->arr[array->occ_size - 1] = NULL;
  array->occ_size--;
  return OP_OK;
}

void remove_task(Task *task) {
  free(task->name);
  free(task->desc);
}

void remove_todo(TodoList *todo) {
  free(todo->name);
  // free(todo->desc);
}

void deinit_task(Task *task) {
  if (task != NULL) {
    free(task->name);
    free(task->desc);
  }
  free(task);
}

void deinit_todo(TodoList *todo) {
  if (todo != NULL) {
    free(todo->name);
    // free(todo->desc);
  }
  free(todo);
}
