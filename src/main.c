#include <ncurses.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "commands.h"
#include "constants.h"
#include "list.h"

typedef enum { IN_PROGESS, DONE } Status;

typedef enum {
  INSERT_MODE,
  NORMAL_MODE,
  SELECT_MODE,
} CursorMode;

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
  Status status;
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

int init_files() {
  struct stat st = {0};
  if (stat("./test", &st)) {
    mkdir("./test", 0700);
  }

  if (stat(TODO_DB_FILE_NAME, &st)) {
    FILE *db_file;
    if (!(db_file = fopen(TODO_DB_FILE_NAME, "w"))) {
      printf("Could not open database file\n");
      fclose(db_file);
      return 1;
    }
    fclose(db_file);
  }
  return 0;
}

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
  if (sqlite3_prepare(db_conn, add_task_query, -1, &add_task_stmt, NULL) !=
      SQLITE_OK) {
    addstr("Can't save the task");
  }

  if (sqlite3_step(add_task_stmt) != SQLITE_DONE) {
    addstr("Can't add task");
  } else {
    // addstr("Added task Successfully");
  }
  sqlite3_finalize(add_task_stmt);
}

void add_task_to_win(WINDOW *win, List *task_list, char *task_name,
                     char *task_desc, int todo_list_id, int *max_cur_pos) {
  Task *task = malloc(sizeof(Task));
  task->name = malloc(strlen(task_name) * sizeof(task->name));
  task->desc = malloc(strlen(task_desc) * sizeof(task->desc));

  strcpy(task->name, task_name);
  strcpy(task->desc, task_desc);
  (*max_cur_pos)++;

  append_list(task_list, task);

  mvwaddstr(win, task_list->occ_size, 30, task_name);
  mvwaddstr(win, task_list->occ_size, 30 + strlen(task_name) + 5, task_desc);
  wrefresh(win);
}

void delete_task_from_win(WINDOW *win, List *task_list, int cur_pos,
                          int *max_cur_pos) {}

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

// TODO: Change so that the version and other information about CNotes are taken
// from constants.h dynamically
void welcome_screen() {
  int rows_number, columns_number;
  getmaxyx(stdscr, rows_number, columns_number);
  char *welcome_msg =
      "CNotes 0.1 VERSION\nCreate notes todos quickly\nThis is just a welcome "
      "page, it will look diffetently in the future";
  clear();
  mvaddstr(rows_number / 2, columns_number / 2, welcome_msg);
  getch();
}

void parse_normal_mode(WINDOW *win, List *todo_list, List *task_list) {
  size_t cur_pos, x;
  short int key = wgetch(win);
  getyx(win, cur_pos, x);
}

/*
void parse_select_mode(WINDOW *win, List *todo_list, List *task_list){

}

void parse_insert_mode(WINDOW *win, List *todo_list, List *task_list){

}

void parse_input(WINDOW *win, CursorMode mode, List *todo_list,
                 List *task_list) {
  size_t cur_pos, x;
  short int key = wgetch(win);
  getyx(win, cur_pos, x);

  if (key == KEY_DOWN) {
    wchgat(win, -1, A_NORMAL, 0, NULL);
    cur_pos++;
    if (cur_pos >) {
      cur_pos = 1;
    }
    wmove(win, cur_pos, 0);
    wchgat(win, -1, A_STANDOUT, 0, NULL);
  } else if (key == KEY_UP) {
    wchgat(win, -1, A_NORMAL, 0, NULL);
    cur_pos--;
    if (cur_pos < 1) {
      cur_pos = list_size;
    }
    wmove(win, cur_pos, 0);
    wchgat(win, -1, A_STANDOUT, 0, NULL);
  }

  if (mode == SELECT_MODE) {

  } else {
  }

  wrefresh(win);
}*/

void notes_screen(sqlite3 *db_conn) {
  int mode = SELECT_MODE;
  WINDOW *tasks_win = newwin(LINES - 1, COLS - 1 - 20, 0, 20);
  WINDOW *list_win = newwin(LINES - 1, 20, 0, 0);
  List *todo_list = get_todo_lists(db_conn);
  List *task_list;
  int cur_pos = 1;
  int list_cur_pos = 1;
  int max_cur_pos = 0;
  int list_max_cur_pos = 0;
  bool are_printed = false;

  clear();
  refresh();
  keypad(list_win, true);
  keypad(tasks_win, true);
  noecho();
  waddstr(list_win, "Todo lists:");

  for (size_t i = 0; i < todo_list->occ_size; i++) {
    TodoList *todo = (TodoList *)get_list(todo_list, i);
    mvwaddstr(list_win, i + 1, 0, todo->name);
  }
  max_cur_pos = todo_list->occ_size;

  wmove(list_win, 1, 0);
  wchgat(list_win, -1, A_STANDOUT, 0, NULL);
  wrefresh(list_win);

  while (true) {
    if (mode == SELECT_MODE) {
      int key = wgetch(list_win);

      if (key == KEY_DOWN) {
        wchgat(list_win, -1, A_NORMAL, 0, NULL);
        cur_pos++;
        if (cur_pos > max_cur_pos) {
          cur_pos = 1;
        }
        wmove(list_win, cur_pos, 0);
        wchgat(list_win, -1, A_STANDOUT, 0, NULL);
      } else if (key == KEY_UP) {
        wchgat(list_win, -1, A_NORMAL, 0, NULL);
        cur_pos--;
        if (cur_pos < 1) {
          cur_pos = max_cur_pos;
        }
        wmove(list_win, cur_pos, 0);
        wchgat(list_win, -1, A_STANDOUT, 0, NULL);
      } else if (key == KEY_RIGHT) {
        wchgat(list_win, -1, A_NORMAL, 0, NULL);
        TodoList *todo = (TodoList *)get_list(todo_list, cur_pos - 1);
        task_list = get_tasks(db_conn, todo->list_id);
        are_printed = false;
        mode = NORMAL_MODE;
      } else if (key == (int)'a') {
        wchgat(list_win, -1, A_NORMAL, 0, NULL);
        mode = NORMAL_MODE;
      }
      wrefresh(list_win);
    }

    else if (mode == NORMAL_MODE) {
      if (!are_printed) {
        for (size_t i = 0; i < task_list->occ_size; i++) {
          Task *task = get_list(task_list, i);
          mvwaddstr(tasks_win, i + 1, 30, task->name);
          mvwaddstr(tasks_win, i + 1, 30 + strlen(task->name) + 5, task->desc);
        }
        list_max_cur_pos = task_list->occ_size;
        wmove(tasks_win, cur_pos, 0);
        wchgat(tasks_win, -1, A_STANDOUT, 0, NULL);
        wrefresh(tasks_win);
        are_printed = true;
      }
      // move(cur_pos, 0);
      // chgat(-1, A_STANDOUT, 0, NULL);

      int key = wgetch(tasks_win);
      if (key == KEY_DOWN) {
        wchgat(tasks_win, -1, A_NORMAL, 0, NULL);
        list_cur_pos++;
        if (list_cur_pos > list_max_cur_pos) {
          list_cur_pos = 1;
        }
        wmove(tasks_win, list_cur_pos, 0);
        wchgat(tasks_win, -1, A_STANDOUT, 0, NULL);
      } else if (key == KEY_UP) {
        wchgat(tasks_win, -1, A_NORMAL, 0, NULL);
        list_cur_pos--;
        if (list_cur_pos < 1) {
          list_cur_pos = list_max_cur_pos;
        }
        wmove(tasks_win, list_cur_pos, 0);
        wchgat(tasks_win, -1, A_STANDOUT, 0, NULL);
      } else if (key == (int)'a') {
        // waddstr(tasks_win, "Added new item");
        wchgat(tasks_win, -1, A_NORMAL, 0, NULL);
        mode = INSERT_MODE;
      } else if (key == (int)'e') {
        waddstr(tasks_win, "Edited item");
        mode = INSERT_MODE;
      } else if (key == (int)'d') {
        waddstr(tasks_win, "Deleted item");
        // delete_task_form_db();
        // delete_task_from_win();
      } else if (key == (int)'x') {
        // TODO: Check whether deinit functions work properly
        deinit_list(task_list, (void (*)(void *)) & deinit_task);
        deinit_list(todo_list, (void (*)(void *)) & deinit_todo);
        return;
      } else if (key == (int)'c') {
        wclear(tasks_win);
        mode = SELECT_MODE;
        list_max_cur_pos = 0;
        list_cur_pos = 1;
        deinit_list(task_list, (void (*)(void *)) & deinit_task);
      } else if (key == 'v') {
        waddstr(tasks_win, "Done");
      }
      wrefresh(tasks_win);
    }

    else if (mode == INSERT_MODE) {
      echo();
      char task_name[TASK_NAME_LEN + 1];
      char task_desc[TASK_DESC_LEN + 1];
      char key;
      WINDOW *new_task_win = newwin(10, COLS - 20 - 1, LINES - 10, 20);

      box(new_task_win, 0, 0);
      wrefresh(new_task_win);
      mvwgetstr(new_task_win, 1, 1, task_name);
      mvwgetstr(new_task_win, 2, 1, task_desc);

      TodoList *todo = (TodoList *)get_list(todo_list, cur_pos - 1);
      add_task_to_db(db_conn, task_name, task_desc, todo->list_id);
      add_task_to_win(tasks_win, task_list, task_name, task_desc, todo->list_id,
                      &list_max_cur_pos);

      wborder(new_task_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
      werase(new_task_win);
      wrefresh(new_task_win);
      delwin(new_task_win);
      noecho();

      mode = NORMAL_MODE;
    }
  }
}

// Function for parsing command line arguments given by the user
int parse_args(int argc, char *argv[]) {
  if (argc < 2)
    return 0;

  for (size_t i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (int j = 1; j < strlen(argv[i]); j++) {
        switch (argv[i][j]) {
        case 'a':
          printf("Creating new todo list\n");
          break;
        default:
          printf("Can't parse argument: %c\n", argv[i][j]);
          return 1;
        }
      }
    } else {
      printf(
          "Can't parse argument: %c. Arguments should look like this: -ajd\n",
          argv[i][0]);
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (init_files()) {
    return 1;
  }

  sqlite3 *db_conn;
  if ((db_conn = init_db()) == NULL) {
    return 1;
  }

  if (parse_args(argc, argv)) {
    return 1;
  }

  initscr();
  curs_set(0);
  keypad(initscr(), true);
  raw();

  welcome_screen();
  notes_screen(db_conn);

  endwin();
  sqlite3_close(db_conn);

  return 0;
}
