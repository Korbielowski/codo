#include "./list.h"


int init_list(List* list){
  list->arr_size = 10;
  list->occ_size = 0;
  list->arr = malloc(list->arr_size*sizeof(void*));
  if(list->arr == NULL){
    return OP_NOT_OK;
  }
  return OP_OK;
}


void deinit_list(List* list, void (*func_p)(void*)){
  if(func_p != NULL){
    for(int i=0; i < list->arr_size; i++){
      printf("destroying index:%d\n", i);
      (*func_p)(list->arr[i]);
    }
  }
  free(list->arr);
}


void* get_list(List* list, size_t index){
  if(index >= list->occ_size){
    return NULL;
  }
  printf("number: %d, at index: %zu\n", *(int*) list->arr[index], index);
  return list->arr[index];
}


int append_list(List* list, void* item){
  list->occ_size++;
  if(list->occ_size > list->arr_size){
    list->arr_size *= 2;
    list->arr = realloc(list->arr, list->arr_size*sizeof(void*));
    if(list->arr == NULL){
      return OP_NOT_OK;
    }
  }
  list->arr[list->occ_size-1] = item;
  return OP_OK;
}


/*int main(){
  
  List list;
  if(init_list(&list) != OP_OK){
    printf("ERROR when initializing a list\n");
  }

  for(int i=0; i < 1000; i++){
    append_list(&list, &i);
    get_list(&list, i);
  }

  deinit_list(&list, NULL);

  return 0;
}*/
