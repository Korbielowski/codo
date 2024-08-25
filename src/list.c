#include "list.h"
#include <stdio.h>
#include <stdlib.h>

int init_list(List *list) {
  list->arr_size = 10;
  list->occ_size = 0;
  list->arr = malloc(list->arr_size * sizeof(void *));
  if (list->arr == NULL) {
    return OP_NOT_OK;
  }
  return OP_OK;
}

void deinit_list(List *list, void (*deinit_func_p)(void *)) {
  if (deinit_func_p != NULL) {
    for (int i = 0; i < list->occ_size; i++) {
      printf("destroying index:%d\n", i);
      (*deinit_func_p)(list->arr[i]);
    }
  }
  free(list->arr);
}

void *get_list(List *list, size_t index) {
  if (index >= list->occ_size) {
    return NULL;
  }
  return list->arr[index];
}

int append_list(List *list, void *item) {
  list->occ_size++;
  if (list->occ_size > list->arr_size) {
    list->arr_size *= 2;
    list->arr = realloc(list->arr, list->arr_size * sizeof(void *));
    if (list->arr == NULL) {
      return OP_NOT_OK;
    }
  }
  list->arr[list->occ_size - 1] = item;
  return OP_OK;
}

int remove_list(List *list, size_t index, void (*remove_func_p)(void *)) {
  (*remove_func_p)(list->arr[index]);
  if (index >= list->occ_size) {
    return OP_NOT_OK;
  }
  for (int i = index; i < list->occ_size - 1; i++) {
    void *element = get_list(list, i + 1);
    if (element == NULL) {
      return OP_NOT_OK;
    }
    list->arr[i] = element;
  }
  list->arr[list->occ_size - 1] = NULL;
  list->occ_size--;
  return OP_OK;
}

void delete_task(Task *task) {
  free(task->name);
  free(task->desc);
}

void delete_todo(TodoList *todo) {
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