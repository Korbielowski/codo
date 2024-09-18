#include <stdio.h>
#include <stdlib.h>

typedef enum { IN_PROGESS, DONE } Status;

typedef enum {
  CREATE_TASK_MODE,
  SELECT_TASK_MODE,
  SELECT_LIST_MODE,
} CursorMode;

typedef struct {
  char *name;
  char *desc;
  int list_id;
  Status status;
} TodoList;

typedef struct {
  char *name;
  char *desc;
  int task_id;
  int list_id;
  Status status;
} Task;
