#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sodium.h>
#include <tomcrypt.h>
#include "util.h"

#define ARG_MAX 1024
//Still working on this function, add to util.h when done
int save(node_t * head, char * name, char * password){
  if(!head){return 1;}
//TODO: add encryption,password salting and hashing to be used as authentication system
  FILE * fp = fopen(name, "w");
  node_t * current = head;
  char * db = NULL;
  int total_size = 0;
  while(current){
    total_size += current->data.uname_l + 1;
    total_size += current->data.ename_l + 1;
    total_size += current->data.pwd_l + 1;
    current = current->next;
  }
  db = malloc(sizeof(char) * (total_size + 1));
  current = head;
  while( current ){
    strncat(db, strcat(current->data.ename, " "), current->data.ename_l + 1);
    strncat(db, strcat(current->data.uname, " "), current->data.uname_l + 1);
    strncat(db, strcat(current->data.pwd, "\n"), current->data.pwd_l + 1);
    current = current->next;
  }
  fwrite(db, 1, total_size, fp);
  return 0;
}

int print_spc(node_t * head, char * enm){
  node_t * current = head;
  while(current){
    if(strncmp(current->data.ename, enm, current->data.ename_l) == 0){
      if(current->data.ename){
        printf("Entryname:   %s  %d\n", current->data.ename, current->data.ename_l);
      }
      if(current->data.uname){
        printf("Username:    %s  %d\n", current->data.uname, current->data.uname_l);
      }
      if(current->data.pwd){
        printf("Password:    %s  %d\n", current->data.pwd, current->data.pwd_l);
      }
      return 0;
    }
    current = current->next;
  }
  return 1;
}

int print_logo(){
  puts(" _  _   ___   _ _   ___   _     _   ___");
  puts("| \\/ | | _ | | \\\\| | _ | | |_  | | |_ _|");
  puts("|_||_| |___| ||| | |___| |___| |_|  |_|\n");
  puts("                  __");
  puts("                 _||__");
  puts("                /     \\");
  puts("               /_______\\");
  puts("               | #   # |");
  puts("               |_______|\n\n");
}
int main(int argc, char **argv)
{  
  char arg_input[ARG_MAX];
  char **args;
  node_t * head = EMPTY_LIST;
  system("clear");
  print_logo();
  while(1){
    printf(">> ");
    fgets( arg_input ,ARG_MAX-1 , stdin);
    arg_input[strlen(arg_input) - 1] = '\0';
    int arg_count = get_arg(arg_input, &args);
    if(arg_count != 0){
//i am aware this is super ugly but ill fix it later
//it works for now

//else if(strcmp(args[0], "name of command" ) == 0 ){
//  if(arg_count == required number of arguments){
//  "whatever function"
//}
//else{puts("Missing arguments");}
//}

//basic command layout
        if(strcmp(args[0], "exit" ) == 0 ){
        free_list(head);
        head = EMPTY_LIST;
	system("clear");
        break;
      }
      else if(strcmp(args[0], "print_list" ) == 0 ){
        print_list(head);
      }

      else if(strcmp(args[0], "new_entry" ) == 0 ){
        if(arg_count == 4 ){
          push(&head, args[1], args[2], args[3]);
	}
	else{puts("Missing arguments");}
      }

      else if(strcmp(args[0], "remove_entry" ) == 0 ){
        if(arg_count == 2 ){
          if(rem_spc(&head, args[1]) == 1 ){
            puts("Entry was not found");
	  }
	}
	else{puts("Missing arguments");}
      }
      else if(strcmp(args[0], "show" ) == 0 ){
        if(arg_count == 2 ){
          if(print_spc(head, args[1]) == 1 ){
            puts("Entry was not found");
	  }
	}
	else{puts("Missing arguments");}
      }
      
      else if(strcmp(args[0], "clear" ) == 0 ){
        if(arg_count == 1){
          system("clear");
	  print_logo();
	}
	else{puts("Missing arguments");}
      }
      else if(strcmp(args[0], "save" ) == 0 ){
        if(arg_count == 3){
	  save(head, args[1] , args[2]);
	}
	else{puts("Missing arguments");}
    }
    else{printf("Unknown command: %s\n", args[0]);}

    for(int i = 0; i < arg_count; ++i){
      sodium_memzero(args[i], strlen(args[i]));
      if(args[i]){free(args[i]);}
    }
  }
  }

  return 0;
}
