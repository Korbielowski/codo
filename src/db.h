#include "array.h"
#include <sqlite3.h>
#include <stdbool.h>

// General database operations
sqlite3 *init_db();
bool check_if_table_exists(sqlite3 *db_conn, char *query);

// Task related operations
int add_task_to_db(sqlite3 *db_conn, char *task_name, char *task_desc,
                   int todo_list_id);
void change_task_status(sqlite3 *db_conn, Task *task);
void delete_task_from_db(sqlite3 *db_conn, int task_id);
Array *get_tasks(sqlite3 *db_conn, int todo_list_id);

// Todo list related operations
Array *get_todo_lists(sqlite3 *db_conn);
int add_todo_list_to_db(sqlite3 *db_conn, char *list_name);
