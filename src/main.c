#include "../include/commands.h"
#include "../include/constants.h"
#include "../include/db.h"
#include <locale.h>
#include <ncurses.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <wchar.h>

const wchar_t TICK[] = L"\u2714";

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

// TODO: Change how this function behaves to make any jump, void
// move_cur_up(WINDOW *win, int *cur_pos, int max_cur_pos, bool from_one, int
// jump) {
void move_cur_up(WINDOW *win, int *cur_pos, int max_cur_pos, bool from_one) {
  short min_pos = from_one ? 1 : 0;
  wchgat(win, -1, A_NORMAL, 0, NULL);
  (*cur_pos)--;
  if (*cur_pos < min_pos) {
    *cur_pos = max_cur_pos;
  }
  wmove(win, *cur_pos, 0);
  wchgat(win, -1, A_STANDOUT, 0, NULL);
}

void move_cur_down(WINDOW *win, int *cur_pos, int max_cur_pos, bool from_one) {
  short min_pos = from_one ? 1 : 0;
  wchgat(win, -1, A_NORMAL, 0, NULL);
  (*cur_pos)++;
  if (*cur_pos > max_cur_pos) {
    *cur_pos = from_one;
  }
  wmove(win, *cur_pos, 0);
  wchgat(win, -1, A_STANDOUT, 0, NULL);
}

void add_todo(WINDOW *win, sqlite3 *db_conn, Array *array, char *name,
              char *desc, int *cur_pos, int *max_cur_pos) {
  TodoList *todo = malloc(sizeof(TodoList));
  todo->name = malloc(strlen(name) * sizeof(todo->name));
  todo->desc = malloc(strlen(desc) * sizeof(todo->desc));

  strcpy(todo->name, name);
  strcpy(todo->desc, desc);

  if (array->occ_size != 0) {
    (*cur_pos)++;
    (*max_cur_pos)++;
  }

  append_array(array, todo);

  mvwaddstr(win, array->occ_size, 0, name);
  wchgat(win, -1, A_STANDOUT, 0, NULL);
  wrefresh(win);

  todo->list_id = add_todo_db(db_conn, name);
}

void delete_todo(WINDOW *win, sqlite3 *db_conn, Array *array, int *cur_pos,
                 int *max_cur_pos) {
  if (array->occ_size == 0) {
    return;
  }

  delete_todo_db(db_conn,
                 ((TodoList *)get_array(array, (*cur_pos) - 1))->list_id);
  remove_array(array, (*cur_pos) - 1, (void (*)(void *)) & remove_todo);
  // TODO: Actually good screen clearing when deleting records
  for (int i = *cur_pos; i < array->occ_size; i++) {
    TodoList *todo = (TodoList *)get_array(array, i);
    wmove(win, i, 0);
    wclrtoeol(win);
    mvwaddstr(win, i, 0, todo->name);
  }
  wmove(win, array->occ_size + 1, 0);
  wclrtoeol(win);

  if (*cur_pos == *max_cur_pos) {
    (*cur_pos)--;
    wmove(win, *cur_pos, 0);
    wchgat(win, -1, A_STANDOUT, 0, NULL);
  }
  (*max_cur_pos)--;

  wmove(win, *cur_pos, 0);
  wchgat(win, -1, A_STANDOUT, 0, NULL);
  wrefresh(win);
}

void change_todo_status(WINDOW *win, sqlite3 *db_conn, Task *task,
                        size_t cur_pos) {
  if (task->status == IN_PROGESS) {
    task->status = DONE;
    mvwprintw(win, cur_pos, COLS - 2, "%ls", TICK);
  } else {
    task->status = IN_PROGESS;
    mvwdelch(win, cur_pos, COLS - 2);
  }
  // TODO: Change color and add checkmark next to done task
  // wchgat(win, -1, A_NORMAL, COLOR_GREEN, NULL);
}

void new_todo_win(sqlite3 *db_conn, Array *array, WINDOW *win, int *cur_pos,
                  int *max_cur_pos) {
  echo();
  char name[TODO_NAME_LEN + 1];
  char desc[TODO_DESC_LEN + 1];
  WINDOW *new_todo_win = newwin(10, COLS - 20 - 1, LINES - 10, 20);

  box(new_todo_win, 0, 0);
  wrefresh(new_todo_win);
  mvwgetstr(new_todo_win, 1, 1, name);
  mvwgetstr(new_todo_win, 2, 1, desc);

  add_todo(win, db_conn, array, name, desc, cur_pos, max_cur_pos);

  wborder(new_todo_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  werase(new_todo_win);
  wrefresh(new_todo_win);
  delwin(new_todo_win);
  noecho();
}

void new_task_win(sqlite3 *db_conn, Array *array) {}

void add_task(WINDOW *win, sqlite3 *db_conn, Array *task_array, char *task_name,
              char *task_desc, int todo_list_id, int *max_cur_pos,
              int *cur_pos) {
  int task_id = add_task_db(db_conn, task_name, task_desc, todo_list_id);
  Task *task = malloc(sizeof(Task));
  task->name = malloc(strlen(task_name) * sizeof(task->name));
  task->desc = malloc(strlen(task_desc) * sizeof(task->desc));
  task->status = IN_PROGESS;
  task->task_id = task_id;

  strcpy(task->name, task_name);
  strcpy(task->desc, task_desc);

  if (task_array->occ_size != 0) {
    (*cur_pos)++;
    (*max_cur_pos)++;
  }

  append_array(task_array, task);

  mvwaddstr(win, task_array->occ_size - 1, 30, task_name);
  mvwaddstr(win, task_array->occ_size - 1, 30 + strlen(task_name) + 5,
            task_desc);
  wchgat(win, -1, A_STANDOUT, 0, NULL);
  wrefresh(win);
}

void delete_task(WINDOW *win, sqlite3 *db_conn, Array *task_array, int *cur_pos,
                 int *max_cur_pos) {
  if (task_array->occ_size == 0) {
    return;
  }
  delete_task_db(db_conn, ((Task *)get_array(task_array, *cur_pos))->task_id);

  remove_array(task_array, *cur_pos, (void (*)(void *)) & remove_task);
  // TODO: Actually good screen clearing when deleting records
  for (int i = *cur_pos; i < task_array->occ_size; i++) {
    Task *task = (Task *)get_array(task_array, i);
    wmove(win, i, 0);
    wclrtoeol(win);
    mvwaddstr(win, i, 30, task->name);
    mvwaddstr(win, i, 30 + strlen(task->name) + 5, task->desc);
  }
  wmove(win, task_array->occ_size, 0);
  wclrtoeol(win);

  if (task_array->occ_size == 0) {
    return;
  }

  if (*cur_pos == *max_cur_pos) {
    (*cur_pos)--;
    wmove(win, *cur_pos, 0);
    wchgat(win, -1, A_STANDOUT, 0, NULL);
  }
  (*max_cur_pos)--;

  wmove(win, *cur_pos, 0);
  wchgat(win, -1, A_STANDOUT, 0, NULL);
  wrefresh(win);
}

void change_task_status(WINDOW *win, sqlite3 *db_conn, Array *array,
                        int cur_pos) {
  if (array->occ_size == 0) {
    return;
  }

  Task *task = (Task *)get_array(array, cur_pos);

  if (task->status == IN_PROGESS) {
    task->status = DONE;
    mvwprintw(win, cur_pos, COLS - 22, "%ls", TICK);
  } else {
    task->status = IN_PROGESS;
    mvwdelch(win, cur_pos, COLS - 22);
  }
  change_task_status_db(db_conn, task);
  // TODO: Change color and add checkmark next to done task
  // wchgat(win, -1, A_NORMAL, COLOR_GREEN, NULL);
}

void print_tasks(WINDOW *tasks_win, Array **task_array, Array *todo_array,
                 sqlite3 *db_conn, int todo_cur_pos) {
  if (todo_array->occ_size == 0) {
    return;
  }

  if (*task_array != NULL) {
    wclear(tasks_win);
    wrefresh(tasks_win);
    deinit_array(*task_array, (void (*)(void *)) & deinit_task);
  }
  TodoList *todo = (TodoList *)get_array(todo_array, todo_cur_pos - 1);
  *task_array = get_tasks(db_conn, todo->list_id);
  for (size_t i = 0; i < (*task_array)->occ_size; i++) {
    Task *task = (Task *)get_array(*task_array, i);
    mvwaddstr(tasks_win, i, 30, task->name);
    mvwaddstr(tasks_win, i, 30 + strlen(task->name) + 5, task->desc);
    if (task->status == DONE) {
      mvwprintw(tasks_win, i, COLS - 22, "%ls", TICK);
    }
  }
  wrefresh(tasks_win);
}

// TODO: Change so that the version and other information about CNotes are
// taken from constants.h dynamically
void welcome_screen() {
  int rows_number, columns_number;
  getmaxyx(stdscr, rows_number, columns_number);
  char *welcome_msg = "CNotes 0.1 VERSION\nCreate notes todos quickly\nThis "
                      "is just a welcome "
                      "page, it will look diffetently in the future";
  clear();
  mvaddstr(rows_number / 2, columns_number / 2, welcome_msg);
  getch();
}

void parse_select_task_mode_input(WINDOW *win, Array *todo_list_array,
                                  Array *task_array) {
  size_t cur_pos, x;
  short int key = wgetch(win);
  getyx(win, cur_pos, x);
}

void notes_screen(sqlite3 *db_conn) {
  int mode = SELECT_LIST_MODE;
  WINDOW *tasks_win = newwin(LINES - 1, COLS - 1 - 20, 0, 20);
  WINDOW *todo_win = newwin(LINES - 1, 20, 0, 0);
  Array *todo_list_array = get_todos(db_conn);
  Array *task_array = NULL;
  int key;
  int todo_cur_pos = 1;
  int task_cur_pos = 0;
  int todo_max_cur_pos, task_max_cur_pos;
  bool are_printed = false;

  clear();
  refresh();
  keypad(todo_win, true);
  keypad(tasks_win, true);
  noecho();
  waddstr(todo_win, "Todo lists:");

  // FixME: after starting app with no todo lists, and adding new one, codo crashes
  if (todo_list_array->occ_size > 0) {
    for (size_t i = 0; i < todo_list_array->occ_size; i++) {
      TodoList *todo = (TodoList *)get_array(todo_list_array, i);
      mvwaddstr(todo_win, i + 1, 0, todo->name);
    }
    todo_max_cur_pos = todo_list_array->occ_size;
    TodoList *todo = (TodoList *)get_array(todo_list_array, todo_cur_pos - 1);
    task_array = get_tasks(db_conn, todo->list_id);
  }

  while (true) {
    if (mode == SELECT_LIST_MODE) {
      // TODO: Add check to not draw this everytime (Same as in the other
      // modes)
      print_tasks(tasks_win, &task_array, todo_list_array, db_conn,
                  todo_cur_pos);
      wmove(todo_win, todo_cur_pos, 0);
      wchgat(todo_win, -1, A_STANDOUT, 0, NULL);
      wrefresh(todo_win);
      key = wgetch(todo_win);

      if (key == KEY_DOWN) {
        move_cur_down(todo_win, &todo_cur_pos, todo_max_cur_pos, true);
      } else if (key == KEY_UP) {
        move_cur_up(todo_win, &todo_cur_pos, todo_max_cur_pos, true);
      } else if (key == KEY_RIGHT) {
        if (todo_list_array->occ_size == 0) {
          continue;
        }
        wchgat(todo_win, -1, A_NORMAL, 0, NULL);
        mode = SELECT_TASK_MODE;
      } else if (key == (int)'a') {
        new_todo_win(db_conn, todo_list_array, todo_win, &todo_cur_pos,
                     &todo_max_cur_pos);
        // add_todo(todo_win, db_conn, todo_list_array, todo_cur_pos);
        //  wchgat(todo_win, -1, A_NORMAL, 0, NULL);
        //  mode = SELECT_TASK_MODE;
      } else if (key == (int)'d') {
        delete_todo(todo_win, db_conn, todo_list_array, &todo_cur_pos,
                    &todo_max_cur_pos);
      }
      wrefresh(todo_win);
    } else if (mode == SELECT_TASK_MODE) {
      if (!are_printed && task_array->occ_size > 0) {
        wclear(tasks_win);
        for (size_t i = 0; i < task_array->occ_size; i++) {
          Task *task = (Task *)get_array(task_array, i);
          mvwaddstr(tasks_win, i, 30, task->name);
          mvwaddstr(tasks_win, i, 30 + strlen(task->name) + 5, task->desc);
          if (task->status == DONE) {
            mvwprintw(tasks_win, i, COLS - 22, "%ls", TICK);
          }
        }

        task_max_cur_pos = task_array->occ_size - 1;
        task_cur_pos = 0;

        wmove(tasks_win, task_cur_pos, 0);
        wchgat(tasks_win, -1, A_STANDOUT, 0, NULL);
        wrefresh(tasks_win);
        are_printed = true;
      }
      // move(cur_pos, 0);
      // chgat(-1, A_STANDOUT, 0, NULL);

      key = wgetch(tasks_win);
      if (key == KEY_DOWN) {
        move_cur_down(tasks_win, &task_cur_pos, task_max_cur_pos, false);
      } else if (key == KEY_UP) {
        move_cur_up(tasks_win, &task_cur_pos, task_max_cur_pos, false);
      } else if (key == (int)'a') {
        wchgat(tasks_win, -1, A_NORMAL, 0, NULL);
        mode = CREATE_TASK_MODE;
      } else if (key == (int)'e') {
        waddstr(tasks_win, "Edited item");
        mode = CREATE_TASK_MODE;
      } else if (key == (int)'d') {
        delete_task(tasks_win, db_conn, task_array, &task_cur_pos,
                    &task_max_cur_pos);
      } else if (key == (int)'x') {
        // TODO: Check whether deinit functions work properly
        deinit_array(task_array, (void (*)(void *)) & deinit_task);
        deinit_array(todo_list_array, (void (*)(void *)) & deinit_todo);
        return;
      } else if (key == (int)'c') {
        wclear(tasks_win);
        mode = SELECT_LIST_MODE;
        // task_max_cur_pos = -1;
        // task_cur_pos = -1;
        are_printed = false;
        deinit_array(task_array, (void (*)(void *)) & deinit_task);
        task_array = NULL;
      } else if (key == 'v') {
        change_task_status(tasks_win, db_conn, task_array, task_cur_pos);
      }
      wrefresh(tasks_win);
    }

    else if (mode == CREATE_TASK_MODE) {
      echo();
      char task_name[TASK_NAME_LEN + 1];
      char task_desc[TASK_DESC_LEN + 1];
      WINDOW *new_task_win = newwin(10, COLS - 20 - 1, LINES - 10, 20);

      box(new_task_win, 0, 0);
      wrefresh(new_task_win);
      mvwgetstr(new_task_win, 1, 1, task_name);
      mvwgetstr(new_task_win, 2, 1, task_desc);

      TodoList *todo = (TodoList *)get_array(todo_list_array, todo_cur_pos - 1);
      add_task(tasks_win, db_conn, task_array, task_name, task_desc,
               todo->list_id, &task_max_cur_pos, &task_cur_pos);

      wborder(new_task_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
      werase(new_task_win);
      wrefresh(new_task_win);
      delwin(new_task_win);
      noecho();

      mode = SELECT_TASK_MODE;
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

  setlocale(LC_ALL, "");
  initscr();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  start_color();
  curs_set(0);
  keypad(initscr(), true);
  raw();

  welcome_screen();
  notes_screen(db_conn);

  endwin();
  sqlite3_close(db_conn);

  return 0;
}
