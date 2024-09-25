#include "array.h"
#include <sqlite3.h>
#include <stdbool.h>

// General database operations
sqlite3 *init_db();
bool check_if_table_exists(sqlite3 *db_conn, char *query);

// Task related operations
int add_task_db(sqlite3 *db_conn, char *task_name, char *task_desc,
                int todo_list_id);
void delete_task_db(sqlite3 *db_conn, int task_id);
void change_task_status_db(sqlite3 *db_conn, Task *task);
Array *get_tasks(sqlite3 *db_conn, int todo_list_id);

// Todo list related operations
int add_todo_db(sqlite3 *db_conn, char *list_name, char* list_desc, Status status);
void delete_todo_db(sqlite3 *db_conn, int todo_id);
void change_todo_status_db(sqlite3 *db_conn, TodoList *todo);
Array *get_todos(sqlite3 *db_conn);
