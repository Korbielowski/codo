#include "list.h"
#include <sqlite3.h>
#include <stdbool.h>

bool check_if_table_exists(sqlite3 *db_conn, char *query);
void add_task_to_db(sqlite3 *db_conn, char *task_name, char *task_desc,
                    int todo_list_id);
void mark_task_as_done_in_db(sqlite3 *db_conn, int task_id);
void delete_task_from_db(sqlite3 *db_conn, int task_id);
List *get_tasks(sqlite3 *db_conn, int todo_list_id);
List *get_todo_lists(sqlite3 *db_conn);
sqlite3 *init_db();