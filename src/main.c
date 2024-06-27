#include <ncurses.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "commands.h"
#include "constants.h"


typedef enum {
  IN_PROGESS,
  DONE
} Status;


typedef enum {
  INSERT_MODE,
  NORMAL_MODE,
  SELECT_MODE,
} CursorMode;


typedef struct{
  char* name;
  char* desc;
  int list_id;
} TodoList;


typedef struct{
  char* name;
  char* desc;
  int task_id;
  int list_id;
  Status status;
} Task;


void de_init(TodoList** todo_list_arr, Task** task_arr){
  size_t i;

  for(i=0; todo_list_arr[i] != NULL; i++){
    free(todo_list_arr[i]->name);
    free(todo_list_arr[i]);
  }
  free(todo_list_arr[i]);
  free(todo_list_arr);

  /*for(i=0; task_arr[i] != NULL; i++){
    free(task_arr[i]->name);
    free(task_arr[i]->desc);
    free(task_arr[i]);
  }
  free(task_arr[i]);
  free(task_arr);
*/
  printf("Deintialization successfully completed\n");
}


int init_files(){
  struct stat st = {0};
  if(stat("./test", &st)){
    mkdir("./test", 0700);
  }

  if(stat(TODO_DB_FILE_NAME, &st)){
    FILE* db_file;
    if(!(db_file = fopen(TODO_DB_FILE_NAME, "w"))){
      printf("Could not open database file\n");
      fclose(db_file);
      return 1;
    }
    fclose(db_file);
  }
  return 0;
}


bool check_if_table_exists(sqlite3* db_conn, char* query){
  sqlite3_stmt* db_stmt;
  char check_tables[400];
  bool exists = false;

  snprintf(check_tables, sizeof(check_tables), "SELECT COUNT (*) FROM sqlite_schema WHERE type='table' AND name = '%s'", query);
  sqlite3_prepare(db_conn, check_tables, -1, &db_stmt, NULL);

  if(sqlite3_step(db_stmt) == SQLITE_ROW && sqlite3_column_int(db_stmt, 0) >= 1){
    printf("Element '%s' already exists\n", query);
    exists = true;
  }

  sqlite3_finalize(db_stmt);

  return exists;
}

void add_task_to_db(sqlite3* db_conn, char* task_name, char* task_desc, int todo_list_id){
  char add_task_query[TASKS_TABLE_NAME_LEN+TASK_DESC_LEN+100];
  sqlite3_stmt* add_task_stmt;

  snprintf(add_task_query, sizeof(add_task_query), "INSERT INTO %s (task_name, task_desc, task_status, list_id) VALUES ('%s', '%s', %d, %d)", TASKS_TABLE_NAME, task_name, task_desc, IN_PROGESS, todo_list_id);
  if(sqlite3_prepare(db_conn, add_task_query, -1, &add_task_stmt, NULL) != SQLITE_OK){
    addstr("Can't save the task");
  }

  if(sqlite3_step(add_task_stmt) != SQLITE_DONE){
    addstr("Can't add task");
  }else {
    addstr("Added task Successfully");
  }
  sqlite3_finalize(add_task_stmt);
}

// Probably this function is useless
/*int get_todo_list_id(sqlite3* db_conn){
  char todo_list_id_query[100+TODO_TABLE_NAME_LENGTH];
  sqlite3_stmt* todo_list_id_stmt;

  snprintf(todo_list_id_query, sizeof(todo_list_id_query), "SELECT ID FROM %s WHERE list_id = %d", TODO_TABLE_NAME, todo_list_id);

  return 0;
}*/

Task** get_tasks(sqlite3* db_conn, int todo_list_id){
  sqlite3_stmt* tasks_stmt;
  char tasks_query[200]; 
  size_t arr_size = 20;
  size_t i = 0;
  Task** task_arr = malloc(arr_size*sizeof(Task*));

  sprintf(tasks_query, "SELECT * FROM %s WHERE list_id = %d", TASKS_TABLE_NAME, todo_list_id);
  if(sqlite3_prepare(db_conn, tasks_query, -1, &tasks_stmt, NULL) != SQLITE_OK){
    addstr("Can't get tasks\n");
  }

  while (sqlite3_step(tasks_stmt) != SQLITE_DONE) {
    Task* task = malloc(sizeof(Task));
    
    task->task_id = sqlite3_column_int(tasks_stmt, 0);

    char* task_name = (char*) sqlite3_column_text(tasks_stmt, 1);
    task->name = malloc(strlen(task_name)*sizeof(task->name));
    strcpy(task->name, task_name);

    char* task_desc = (char*) sqlite3_column_text(tasks_stmt, 2);
    task->desc = malloc(strlen(task_desc)*sizeof(task->desc));
    strcpy(task->desc, task_desc);

    task->status = sqlite3_column_int(tasks_stmt, 3);

    task->list_id = sqlite3_column_int(tasks_stmt, 4);

    if(i > arr_size){
      arr_size *= 2;
      task_arr = realloc(task_arr, arr_size*sizeof(Task*)); 
    }

    task_arr[i] = task;
    i++;
  }

  if(i > arr_size){
    arr_size++;
    task_arr = realloc(task_arr, (arr_size+1)*sizeof(Task*));
  }

  task_arr[i] = NULL;

  sqlite3_finalize(tasks_stmt);

  return task_arr;
}


TodoList** get_todo_lists(sqlite3* db_conn){
  sqlite3_stmt* todo_lists_stmt;
  char todo_lists_query[200];
  size_t arr_size = 10;
  size_t i = 0;
  TodoList** todo_list_arr = malloc(arr_size*sizeof(TodoList*));
 
  sprintf(todo_lists_query, "SELECT * FROM %s", TODO_TABLE_NAME);
  if(sqlite3_prepare(db_conn, todo_lists_query, -1, &todo_lists_stmt, NULL) != SQLITE_OK){
    addstr("Can't get todo lists\n");
  }

  while(sqlite3_step(todo_lists_stmt) != SQLITE_DONE){
    TodoList* todo_list = malloc(sizeof(TodoList));

    todo_list->list_id = sqlite3_column_int(todo_lists_stmt, 0);

    char* list_name = (char*) sqlite3_column_text(todo_lists_stmt, 1);
    todo_list->name = malloc(strlen(list_name)*sizeof(todo_list->name));
    strcpy(todo_list->name, list_name);
    
    if(i > arr_size){
      arr_size *= 2;
      todo_list_arr = realloc(todo_list_arr, arr_size*sizeof(TodoList*));
    }

    todo_list_arr[i] = todo_list;
    i++;
  }

  if(i > arr_size){
    arr_size++;
    todo_list_arr = realloc(todo_list_arr, (arr_size+1)*sizeof(Task*));
  }

  todo_list_arr[i] = NULL;

  sqlite3_finalize(todo_lists_stmt);

  return todo_list_arr;
}


sqlite3* init_db(){
  sqlite3* db_conn;
  char* db_error_msg;

  if(sqlite3_open(TODO_DB_FILE_NAME, &db_conn)){
    printf("Can't open the database\n");
    sqlite3_close(db_conn);
    return NULL;
  }

  if(!check_if_table_exists(db_conn, TODO_TABLE_NAME)){
    char create_todo_table[300];
    snprintf(create_todo_table, sizeof(create_todo_table), "CREATE TABLE %s (list_id INTEGER PRIMARY KEY, list_name VARCHAR(100) NOT NULL)", TODO_TABLE_NAME);
    if(sqlite3_exec(db_conn, create_todo_table, NULL, 0, &db_error_msg) != SQLITE_OK){
      printf("Can't create the todo table: %s\n", db_error_msg);
      return NULL;
    }else{
      printf("Successfully created '%s' table\n", TODO_TABLE_NAME);
    }
  }
  
  if(!check_if_table_exists(db_conn, TASKS_TABLE_NAME)){
    char create_tasks_table[400];
    snprintf(create_tasks_table, sizeof(create_tasks_table),\
              "CREATE TABLE %s (task_id INTEGER PRIMARY KEY, list_id INTEGER NOT NULL,\
              task_name VARCHAR(100) NOT NULL,\
              task_desc VARCHAR(200),\
              task_status INTEGER NOT NULL, \
              FOREIGN KEY (list_id) REFERENCES %s (list_id))",\
              TASKS_TABLE_NAME, TODO_TABLE_NAME);
    if(sqlite3_exec(db_conn, create_tasks_table, NULL, 0, &db_error_msg) != SQLITE_OK){
      printf("Can't create the tasks table: %s\n", db_error_msg);
      return NULL;
    }else{
      printf("Successfully created '%s' table\n", TASKS_TABLE_NAME);
    }
  }
  return db_conn;
}


//TODO: Change so that the version and other information about CNotes are taken from constants.h dynamically 
void welcome_screen(){
  int rows_number, columns_number;
  getmaxyx(stdscr, rows_number, columns_number);
  char* welcome_msg = "CNotes 0.1 VERSION\nCreate notes todos quickly\nThis is just a welcome page, it will look diffetently in the future";
  clear();
  mvaddstr(rows_number/2, columns_number/2, welcome_msg);
  getch();
}


void notes_screen(sqlite3* db_conn){
  char normal_mode_input[3] = {'\0'};
  int mode = SELECT_MODE;
  WINDOW* item_creation_window;
  WINDOW* list_selection_window = newwin(LINES-1, 20, 0, 0);
  TodoList** todo_list_arr = get_todo_lists(db_conn);
  Task** task_arr;
  int cur_pos = 1;
  int max_cur_pos = 0;

  waddstr(list_selection_window, "Todo lists:");

  for(size_t i=0; todo_list_arr[i] != NULL; i++){
    mvwaddstr(list_selection_window, i+1, 0, todo_list_arr[i]->name);
    max_cur_pos++;
  }

  wmove(list_selection_window, cur_pos, 0);
  wchgat(list_selection_window, -1, A_STANDOUT, 0, NULL);
  wrefresh(list_selection_window);

  while(true){
    if(mode == SELECT_MODE){
      noecho();
      int key = getch(); 
      
      if(key == KEY_DOWN){
        wchgat(list_selection_window, -1, A_NORMAL, 0, NULL);
        cur_pos++;
        if(cur_pos > max_cur_pos){
          cur_pos = 1;
        }
        wmove(list_selection_window, cur_pos, 0);
        wchgat(list_selection_window, -1, A_STANDOUT, 0, NULL);
      }else if(key == KEY_UP){
        wchgat(list_selection_window, -1, A_NORMAL, 0, NULL);
        cur_pos--;
        if(cur_pos < 1){
          cur_pos = max_cur_pos;
        }
        wmove(list_selection_window, cur_pos, 0);
        wchgat(list_selection_window, -1, A_STANDOUT, 0, NULL);
      } else {
        wchgat(list_selection_window, -1, A_NORMAL, 0, NULL);
        mode = NORMAL_MODE; 
      }
      wrefresh(list_selection_window);
    }else if(mode == NORMAL_MODE){
      noecho();
      getnstr(normal_mode_input, 2);

      if(strcmp(normal_mode_input, ADD_NEW_ITEM) == 0){
        addstr("Added new item");
        mode = INSERT_MODE;
      }else if(strcmp(normal_mode_input, EDIT_ITEM) == 0){
        addstr("Edited item");
        mode = INSERT_MODE;
      }else if(strcmp(normal_mode_input, DELETE_ITEM) == 0){
        addstr("Deleted item");
      }else if(strcmp(normal_mode_input, EXIT_CNOTES) == 0){
        de_init(todo_list_arr, task_arr);
        return;
      }else{
        addstr("Bad input");
        continue;
      }
    }else if(mode == INSERT_MODE){
      echo();
      char task_name[TASK_NAME_LEN+1];
      char task_desc[TASK_DESC_LEN+1];
      char key;
      item_creation_window = newwin(5, 10, LINES - 10, COLS - 20);

      box(item_creation_window, 0, 0);
      wrefresh(item_creation_window);
      wgetstr(item_creation_window, task_name);
      wgetstr(item_creation_window, task_desc);

      //add_task_to_db(db_conn, task_name, task_desc, );
      
      waddstr(item_creation_window, "Added item:");

      mode = NORMAL_MODE;
    }
    //refresh();
  }
}


// Function for parsing command line arguments given by the user
int parse_args(int argc, char* argv[]){
  if(argc < 2)
    return 0;

  for(size_t i=1; i<argc; i++){
    if(argv[i][0] == '-'){
      for(int j=1; j<strlen(argv[i]); j++){
        switch (argv[i][j]) {
          case 'a':
            printf("Creating new todo list\n");
            break;
          default:
            printf("Can't parse argument: %c\n", argv[i][j]);
            return 1;
        }
      }
    }else{
      printf("Can't parse argument: %c. Arguments should look like this: -ajd\n", argv[i][0]);
      return 1;
    }
  }
  return 0;
}

int main(int argc, char* argv[]){
  if(init_files()){
    return 1;
  }

  sqlite3* db_conn;
  if((db_conn = init_db()) == NULL){
    return 1;
  }

  if(parse_args(argc, argv)){
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
