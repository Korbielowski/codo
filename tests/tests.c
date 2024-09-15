#include "../include/array.h"
#include <stdio.h>
#include <string.h>

int main() {
  Array *dyn_array = malloc(sizeof(Array));
  // init_array(dyn_array);
  // for (int i = 0; i < 10; i++) {
  //   Task *task = malloc(sizeof(Task));
  //   task->name = malloc(4 * sizeof(char));
  //   task->desc = malloc(8 * sizeof(char));
  //   strcpy(task->name, "test name");
  //   strcpy(task->desc, "test desc");
  //   append_array(dyn_array, task);
  // }
  //
  // for (int i = 0; i < dyn_array->occ_size; i++) {
  //   printf("Element %d: %s\n", i, ((Task *)get_array(dyn_array, i))->name);
  // }
  //
  // size_t len = dyn_array->occ_size;
  // for (int i = 0; i < len; i++) {
  //   printf("Remove element: %d\n", i);
  //   remove_array(dyn_array, 0, (void (*)(void *)) & delete_task);
  // }
  //
  // for (int i = 0; i < dyn_array->occ_size; i++) {
  //   printf("After removal %d: %s\n", i,
  //          ((Task *)get_array(dyn_array, i))->name);
  // }
  //
  // deinit_array(dyn_array, (void (*)(void *)) & deinit_task);
  // free(dyn_array);
}
