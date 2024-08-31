#include "commands.h"
#include "constants.h"
#include "db.h"
#include <locale.h>
#include <ncurses.h>
#include <sqlite3.h>
#include <stdbool.h>
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

void add_task_to_win(WINDOW *win, Array *task_array, char *task_name,
                     char *task_desc, int todo_list_id, int task_id,
                     int *max_cur_pos, int *cur_pos) {
  Task *task = malloc(sizeof(Task));
  task->name = malloc(strlen(task_name) * sizeof(task->name));
  task->desc = malloc(strlen(task_desc) * sizeof(task->desc));
  task->status = IN_PROGESS;
  task->task_id = task_id;

  strcpy(task->name, task_name);
  strcpy(task->desc, task_desc);
  (*max_cur_pos)++;
  (*cur_pos)++;

  append_array(task_array, task);

  mvwaddstr(win, task_array->occ_size - 1, 30, task_name);
  mvwaddstr(win, task_array->occ_size - 1, 30 + strlen(task_name) + 5,
            task_desc);
  wchgat(win, -1, A_STANDOUT, 0, NULL);
  wrefresh(win);
}

void delete_task_from_win(WINDOW *win, Array *task_array, int *cur_pos,
                          int *max_cur_pos) {
  remove_array(task_array, *cur_pos, (void (*)(void *)) & delete_task);
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

void change_task_status_win(WINDOW *win, Task *task, size_t cur_pos) {
  if (task->status == IN_PROGESS) {
    task->status = DONE;
    mvwprintw(win, cur_pos, COLS - 22, "%ls", TICK);
  } else {
    task->status = IN_PROGESS;
    mvwdelch(win, cur_pos, COLS - 22);
  }
  // TODO: Change color and add checkmark next to done task
  // wchgat(win, -1, A_NORMAL, COLOR_GREEN, NULL);
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

void parse_select_task_mode_input(WINDOW *win, Array *todo_list_array,
                                  Array *task_array) {
  size_t cur_pos, x;
  short int key = wgetch(win);
  getyx(win, cur_pos, x);
}

void notes_screen(sqlite3 *db_conn) {
  int mode = SELECT_LIST_MODE;
  WINDOW *tasks_win = newwin(LINES - 1, COLS - 1 - 20, 0, 20);
  WINDOW *list_win = newwin(LINES - 1, 20, 0, 0);
  Array *todo_list_array = get_todo_lists(db_conn);
  Array *task_array;
  int key;
  int todo_cur_pos = 1;
  int task_cur_pos = 0;
  int todo_max_cur_pos, task_max_cur_pos;
  bool are_printed = false;

  clear();
  refresh();
  keypad(list_win, true);
  keypad(tasks_win, true);
  noecho();
  // wchar_t characters[] = L"\u0837";
  // const char martini[5] = {0xF0, 0x9F, 0x98, 0x81, '\0'};
  // wprintw(list_win, "%ls :", characters);
  waddstr(list_win, "Todo lists:");

  for (size_t i = 0; i < todo_list_array->occ_size; i++) {
    TodoList *todo = (TodoList *)get_array(todo_list_array, i);
    mvwaddstr(list_win, i + 1, 0, todo->name);
  }
  todo_max_cur_pos = todo_list_array->occ_size;

  while (true) {
    if (mode == SELECT_LIST_MODE) {
      // TODO: Add check to not draw this everytime (Same as in the other modes)
      wmove(list_win, todo_cur_pos, 0);
      wchgat(list_win, -1, A_STANDOUT, 0, NULL);
      wrefresh(list_win);
      key = wgetch(list_win);

      if (key == KEY_DOWN) {
        move_cur_down(list_win, &todo_cur_pos, todo_max_cur_pos, true);
      } else if (key == KEY_UP) {
        move_cur_up(list_win, &todo_cur_pos, todo_max_cur_pos, true);
      } else if (key == KEY_RIGHT) {
        wchgat(list_win, -1, A_NORMAL, 0, NULL);
        TodoList *todo =
            (TodoList *)get_array(todo_list_array, todo_cur_pos - 1);
        task_array = get_tasks(db_conn, todo->list_id);
        are_printed = false;
        mode = SELECT_TASK_MODE;
      } else if (key == (int)'a') {
        // wchgat(list_win, -1, A_NORMAL, 0, NULL);
        // mode = SELECT_TASK_MODE;
      }
      wrefresh(list_win);
    } else if (mode == SELECT_TASK_MODE) {
      if (!are_printed) {
        for (size_t i = 0; i < task_array->occ_size; i++) {
          Task *task = (Task *)get_array(task_array, i);
          mvwaddstr(tasks_win, i, 30, task->name);
          mvwaddstr(tasks_win, i, 30 + strlen(task->name) + 5, task->desc);
          if (task->status == DONE) {
            mvwprintw(tasks_win, i, COLS - 22, "%ls", TICK);
          }
        }

        task_max_cur_pos = task_array->occ_size - 1;
        if (task_cur_pos > task_max_cur_pos) {
          task_cur_pos = 0;
        }

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
        // waddstr(tasks_win, "Added new item");
        wchgat(tasks_win, -1, A_NORMAL, 0, NULL);
        mode = CREATE_TASK_MODE;
      } else if (key == (int)'e') {
        waddstr(tasks_win, "Edited item");
        mode = CREATE_TASK_MODE;
      } else if (key == (int)'d') {
        if (task_array->occ_size == 0) {
          continue;
        }
        delete_task_from_db(
            db_conn, ((Task *)get_array(task_array, task_cur_pos))->task_id);
        delete_task_from_win(tasks_win, task_array, &task_cur_pos,
                             &task_max_cur_pos);
      } else if (key == (int)'x') {
        // TODO: Check whether deinit functions work properly
        deinit_array(task_array, (void (*)(void *)) & deinit_task);
        deinit_array(todo_list_array, (void (*)(void *)) & deinit_todo);
        return;
      } else if (key == (int)'c') {
        wclear(tasks_win);
        mode = SELECT_LIST_MODE;
        task_max_cur_pos = 0;
        task_cur_pos = 0;
        deinit_array(task_array, (void (*)(void *)) & deinit_task);
      } else if (key == 'v') {
        change_task_status_win(tasks_win,
                               (Task *)get_array(task_array, task_cur_pos),
                               task_cur_pos);
        change_task_status(db_conn,
                           ((Task *)get_array(task_array, task_cur_pos)));
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
      int task_id =
          add_task_to_db(db_conn, task_name, task_desc, todo->list_id);
      add_task_to_win(tasks_win, task_array, task_name, task_desc,
                      todo->list_id, task_id, &task_max_cur_pos, &task_cur_pos);

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
