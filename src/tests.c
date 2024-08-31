#include "array.h"
#include <stdio.h>

typedef struct {
  int x;
} Number;

void deinit_number(Number *num) { free(num); }

void remove_number(Number *num) { return; }

typedef struct {
  char *name;
  char *desc;
  int list_id;
} TodoList;

typedef struct {
  char *name;
  char *desc;
  int task_id;
  int list_id;
  // Status status;
} Task;

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

int main() {
  Array *dyn_array = malloc(sizeof(Array));
  init_list(dyn_array);
  for (int i = 0; i < 10; i++) {
    Task *task = malloc(sizeof(Task));
    task->name = malloc(4 * sizeof(char));
    task->desc = malloc(8 * sizeof(char));
    strcpy(task->name, "xdd");
    strcpy(task->desc, "cos tam");
    append_list(dyn_array, task);
  }

  for (int i = 0; i < dyn_array->occ_size; i++) {
    printf("Element %d: %s\n", i, ((Task *)get_list(dyn_array, i))->name);
  }

  size_t len = dyn_array->occ_size;
  for (int i = 0; i < len; i++) {
    printf("Remove element: %d\n", i);
    remove_list(dyn_array, 0, (void (*)(void *)) & delete_task);
  }

  for (int i = 0; i < dyn_array->occ_size; i++) {
    printf("After removal %d: %s\n", i, ((Task *)get_list(dyn_array, i))->name);
  }

  deinit_list(dyn_array, (void (*)(void *)) & deinit_task);
  // free(dyn_array);
}
