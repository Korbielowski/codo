#include "db.h"
#include "constants.h"
#include <ncurses.h>
#include <string.h>

bool check_if_table_exists(sqlite3 *db_conn, char *query) {
  sqlite3_stmt *db_stmt;
  char check_tables[400];
  bool exists = false;

  snprintf(
      check_tables, sizeof(check_tables),
      "SELECT COUNT (*) FROM sqlite_schema WHERE type='table' AND name = '%s'",
      query);
  sqlite3_prepare(db_conn, check_tables, -1, &db_stmt, NULL);

  if (sqlite3_step(db_stmt) == SQLITE_ROW &&
      sqlite3_column_int(db_stmt, 0) >= 1) {
    printf("Element '%s' already exists\n", query);
    exists = true;
  }

  sqlite3_finalize(db_stmt);

  return exists;
}

void add_task_to_db(sqlite3 *db_conn, char *task_name, char *task_desc,
                    int todo_list_id) {
  char add_task_query[TASKS_TABLE_NAME_LEN + TASK_DESC_LEN + 100];
  sqlite3_stmt *add_task_stmt;

  snprintf(add_task_query, sizeof(add_task_query),
           "INSERT INTO %s (task_name, task_description, list_id) VALUES "
           "('%s', '%s', %d)",
           TASKS_TABLE_NAME, task_name, task_desc, todo_list_id);
  FILE *file;
  file = fopen("filename.txt", "a");
  fprintf(file, "values saved: %s %s %d %d\n", task_name, task_desc,
          todo_list_id, IN_PROGESS);
  fclose(file);
  if (sqlite3_prepare(db_conn, add_task_query, -1, &add_task_stmt, NULL) !=
      SQLITE_OK) {
    addstr("Can't prepare task addition statement");
  }

  if (sqlite3_step(add_task_stmt) != SQLITE_DONE) {
    addstr("Can't add task");
  }
  sqlite3_finalize(add_task_stmt);
}

// ! Fix bug where deleted task is not saved in db for some reason
void delete_task_from_db(sqlite3 *db_conn, int task_id) {
  char delete_task_query[TASKS_TABLE_NAME_LEN + 50];
  sqlite3_stmt *delete_task_stmt;

  snprintf(delete_task_query, sizeof(delete_task_query),
           "DELETE FROM %s WHERE task_id = %d", TASKS_TABLE_NAME, task_id);
  if (sqlite3_prepare(db_conn, delete_task_query, -1, &delete_task_stmt,
                      NULL) != SQLITE_OK) {
    addstr("Can't prepare delete statement");
  }

  if (sqlite3_step(delete_task_stmt) != SQLITE_DONE) {
    addstr("Can't delete item from database");
  }
  sqlite3_finalize(delete_task_stmt);
}

void change_task_status(sqlite3 *db_conn, Task *task) {
  sqlite3_stmt *mark_as_done_stmt;
  char mark_as_done_query[200];
  snprintf(mark_as_done_query, sizeof(mark_as_done_query),
           "UPDATE %s SET status = %d WHERE task_id = %d", TASKS_TABLE_NAME,
           task->status, task->task_id);
  if (sqlite3_prepare(db_conn, mark_as_done_query, -1, &mark_as_done_stmt,
                      NULL) != SQLITE_OK) {
    addstr("Cannot prepare update statement");
  }

  if (sqlite3_step(mark_as_done_stmt) != SQLITE_DONE) {
    addstr("Cannot update task");
  }
  sqlite3_finalize(mark_as_done_stmt);
  FILE *file;
  file = fopen("filename.txt", "a");
  fprintf(file, "write status: %d\n", task->status);
  fclose(file);
}

List *get_tasks(sqlite3 *db_conn, int todo_list_id) {
  sqlite3_stmt *tasks_stmt;
  char tasks_query[200];
  List *task_list = malloc(sizeof(List));

  init_list(task_list);

  sprintf(tasks_query, "SELECT * FROM %s WHERE list_id = %d", TASKS_TABLE_NAME,
          todo_list_id);
  if (sqlite3_prepare(db_conn, tasks_query, -1, &tasks_stmt, NULL) !=
      SQLITE_OK) {
    addstr("Can't get tasks\n");
  }

  while (sqlite3_step(tasks_stmt) != SQLITE_DONE) {
    Task *task = malloc(sizeof(Task));

    task->task_id = sqlite3_column_int(tasks_stmt, 0);
    task->list_id = sqlite3_column_int(tasks_stmt, 1);

    char *task_name = (char *)sqlite3_column_text(tasks_stmt, 2);
    task->name = malloc(strlen(task_name) * sizeof(task->name));
    strcpy(task->name, task_name);

    char *task_desc = (char *)sqlite3_column_text(tasks_stmt, 3);
    task->desc = malloc(strlen(task_desc) * sizeof(task->desc));
    strcpy(task->desc, task_desc);

    task->status = sqlite3_column_int(tasks_stmt, 4);

    FILE *file;
    file = fopen("filename.txt", "a");
    fprintf(file, "read status: %d\n", task->status);
    fclose(file);

    append_list(task_list, task);
  }

  sqlite3_finalize(tasks_stmt);

  return task_list;
}

List *get_todo_lists(sqlite3 *db_conn) {
  sqlite3_stmt *todo_lists_stmt;
  char todo_lists_query[200];
  List *todo_list = malloc(sizeof(List));

  init_list(todo_list);

  sprintf(todo_lists_query, "SELECT * FROM %s", TODO_TABLE_NAME);
  if (sqlite3_prepare(db_conn, todo_lists_query, -1, &todo_lists_stmt, NULL) !=
      SQLITE_OK) {
    addstr("Can't get todo lists\n");
  }

  while (sqlite3_step(todo_lists_stmt) != SQLITE_DONE) {
    TodoList *todo = malloc(sizeof(TodoList));

    todo->list_id = sqlite3_column_int(todo_lists_stmt, 0);

    char *list_name = (char *)sqlite3_column_text(todo_lists_stmt, 1);
    todo->name = malloc(strlen(list_name) * sizeof(todo->name));
    strcpy(todo->name, list_name);

    append_list(todo_list, todo);
  }

  sqlite3_finalize(todo_lists_stmt);

  return todo_list;
}

sqlite3 *init_db() {
  sqlite3 *db_conn;
  char *db_error_msg;

  if (sqlite3_open(TODO_DB_FILE_NAME, &db_conn)) {
    printf("Can't open the database\n");
    sqlite3_close(db_conn);
    return NULL;
  }

  if (!check_if_table_exists(db_conn, TODO_TABLE_NAME)) {
    char create_todo_table[300];
    snprintf(create_todo_table, sizeof(create_todo_table),
             "CREATE TABLE %s (list_id INTEGER PRIMARY KEY, list_name "
             "VARCHAR(100) NOT NULL)",
             TODO_TABLE_NAME);
    if (sqlite3_exec(db_conn, create_todo_table, NULL, 0, &db_error_msg) !=
        SQLITE_OK) {
      printf("Can't create the todo table: %s\n", db_error_msg);
      return NULL;
    } else {
      printf("Successfully created '%s' table\n", TODO_TABLE_NAME);
    }
  }

  if (!check_if_table_exists(db_conn, TASKS_TABLE_NAME)) {
    char create_tasks_table[400];
    snprintf(
        create_tasks_table, sizeof(create_tasks_table),
        "CREATE TABLE %s (task_id INTEGER PRIMARY KEY, list_id INTEGER NOT NULL,\
              task_name VARCHAR(100) NOT NULL,\
              task_desc VARCHAR(200),\
              task_status INTEGER NOT NULL, \
              FOREIGN KEY (list_id) REFERENCES %s (list_id))",
        TASKS_TABLE_NAME, TODO_TABLE_NAME);
    if (sqlite3_exec(db_conn, create_tasks_table, NULL, 0, &db_error_msg) !=
        SQLITE_OK) {
      printf("Can't create the tasks table: %s\n", db_error_msg);
      return NULL;
    } else {
      printf("Successfully created '%s' table\n", TASKS_TABLE_NAME);
    }
  }
  return db_conn;
}