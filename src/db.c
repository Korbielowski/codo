#include "../include/db.h"
#include "../include/array.h"
#include "../include/constants.h"
#include <ncurses.h>
#include <stdio.h>
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

int add_task_to_db(sqlite3 *db_conn, char *task_name, char *task_desc,
                   int todo_list_id) {
  char add_task_query[TASKS_TABLE_NAME_LEN + TASK_DESC_LEN + 100];
  sqlite3_stmt *add_task_stmt;
  int task_id;

  snprintf(add_task_query, sizeof(add_task_query),
           "INSERT INTO %s (task_name, task_desc, list_id, task_status) "
           "VALUES "
           "('%s', '%s', %d, %d)",
           TASKS_TABLE_NAME, task_name, task_desc, todo_list_id, IN_PROGESS);
  if (sqlite3_prepare_v2(db_conn, add_task_query, -1, &add_task_stmt, NULL) !=
      SQLITE_OK) {
    addstr("Can't prepare task addition statement");
  }

  if (sqlite3_step(add_task_stmt) != SQLITE_DONE) {
    addstr("Can't add task");
  }
  sqlite3_finalize(add_task_stmt);

  char get_task_id_query[TASKS_TABLE_NAME_LEN + TASKS_TABLE_NAME_LEN + 100];
  sqlite3_stmt *get_task_id_stmt;
  snprintf(get_task_id_query, sizeof(get_task_id_query),
           "SELECT * FROM %s WHERE task_id = (SELECT MAX(task_id) FROM %s)",
           TASKS_TABLE_NAME, TASKS_TABLE_NAME);
  if (sqlite3_prepare_v2(db_conn, get_task_id_query, -1, &get_task_id_stmt,
                         NULL) != SQLITE_OK) {
    addstr("Can't prepare task addition statement");
  }

  if (sqlite3_step(get_task_id_stmt) == SQLITE_DONE) {
    addstr("Can't add task");
  } else {
    task_id = sqlite3_column_int(get_task_id_stmt, 0);
  }

  FILE *file;
  file = fopen("data.txt", "a");
  fprintf(file, "Task id: %d\nQuery: %s\n\n", task_id, add_task_query);
  fclose(file);

  sqlite3_finalize(get_task_id_stmt);
  return task_id;
}

int add_todo_list_to_db(sqlite3 *db_conn, char *list_name) {
  char add_todo_list_query[TODO_TABLE_NAME_LEN + 100];
  sqlite3_stmt *add_todo_list_stmt;
  int todo_list_id;

  snprintf(add_todo_list_query, sizeof(add_todo_list_query),
           "INSERT INTO %s (list_name) VALUES ('%s')", TODO_TABLE_NAME,
           list_name);
  if (sqlite3_prepare_v2(db_conn, add_todo_list_query, -1, &add_todo_list_stmt,
                         NULL) != SQLITE_OK) {
    addstr("Can't prepare task addition statement");
  }

  if (sqlite3_step(add_todo_list_stmt) != SQLITE_DONE) {
    addstr("Can't add task");
  }
  sqlite3_finalize(add_todo_list_stmt);

  char get_todo_list_id_query[TODO_TABLE_NAME_LEN * 2 + 100];
  sqlite3_stmt *get_todo_list_id_stmt;
  snprintf(get_todo_list_id_query, sizeof(get_todo_list_id_query),
           "SELECT * FROM %s WHERE list_id = (SELECT MAX(list_id) FROM %s)",
           TODO_TABLE_NAME, TODO_TABLE_NAME);
  if (sqlite3_prepare_v2(db_conn, get_todo_list_id_query, -1,
                         &get_todo_list_id_stmt, NULL) != SQLITE_OK) {
    addstr("Can't prepare task addition statement");
  }

  if (sqlite3_step(get_todo_list_id_stmt) == SQLITE_DONE) {
    addstr("Can't add task");
  } else {
    todo_list_id = sqlite3_column_int(get_todo_list_id_stmt, 0);
  }
  sqlite3_finalize(get_todo_list_id_stmt);
  return todo_list_id;
}

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
           "UPDATE %s SET task_status = %d WHERE task_id = %d",
           TASKS_TABLE_NAME, task->status, task->task_id);
  if (sqlite3_prepare(db_conn, mark_as_done_query, -1, &mark_as_done_stmt,
                      NULL) != SQLITE_OK) {
    addstr("Cannot prepare update statement");
  }

  if (sqlite3_step(mark_as_done_stmt) != SQLITE_DONE) {
    addstr("Cannot update task");
  }
  sqlite3_finalize(mark_as_done_stmt);
}

Array *get_tasks(sqlite3 *db_conn, int todo_list_id) {
  sqlite3_stmt *tasks_stmt;
  char tasks_query[200];
  Array *task_array = malloc(sizeof(Array));

  init_array(task_array);

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

    append_array(task_array, task);
  }

  sqlite3_finalize(tasks_stmt);

  return task_array;
}

Array *get_todo_lists(sqlite3 *db_conn) {
  sqlite3_stmt *todo_lists_stmt;
  char todo_lists_query[200];
  Array *todo_list_array = malloc(sizeof(Array));

  init_array(todo_list_array);

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

    append_array(todo_list_array, todo);
  }

  sqlite3_finalize(todo_lists_stmt);

  return todo_list_array;
}

sqlite3 *init_db() {
  sqlite3 *db_conn;
  char *db_error_msg;

  if (sqlite3_open_v2(TODO_DB_FILE_NAME, &db_conn,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
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
