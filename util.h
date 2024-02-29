#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include <stdbool.h>

#define EMPTY_LIST NULL

typedef struct entry{
  char * ename;
  char * uname;
  char * pwd;
  int ename_l;
  int uname_l;
  int pwd_l;
}etr;

typedef struct node_t{
  etr data;
  struct node_t * next;
}node_t;

int push(node_t ** head, char * enm, char * unm, char * pwd) {
//setting up new node and transferring data
  node_t * new_node = malloc(sizeof(node_t));  
  if(new_node){
    new_node->data.ename_l = strlen(enm);
    new_node->data.uname_l = strlen(unm);
    new_node->data.pwd_l = strlen(pwd);
    }
  else{return 1;}
//calloc is used here cus malloc was doing a wierd
  new_node->data.ename = calloc(1 , sizeof(char) * new_node->data.ename_l);  
  new_node->data.uname = calloc(1, sizeof(char) * new_node->data.uname_l);  
  new_node->data.pwd = calloc(1, sizeof(char) * new_node->data.pwd_l);  

  if(new_node->data.ename && new_node->data.uname && new_node->data.pwd){
    strncpy(new_node->data.ename, enm, new_node->data.ename_l);
    strncpy(new_node->data.uname, unm, new_node->data.uname_l);
    strncpy(new_node->data.pwd, pwd, new_node->data.pwd_l);
  }

  //pushing new node to the list
  new_node->next = *head;
  *head = new_node;
  return 0;
}

int entry_exists(node_t * head, char * entry_name){
  if(!head){return 1;}
  node_t * current = head;
  while(current){
    if(strcmp(current->data.ename, entry_name)){
      return 0;
    }
    current = current->next;
  }
  return 1;
}

int print_list(node_t * head) {
//prints all entry values
  if(!head){return 1;}
  node_t * current = head;
  while (current) {
    if(current->data.ename){
      printf("Entryname:   %s  %d\n", current->data.ename, current->data.ename_l);
    }		
    if(current->data.uname){
      printf("Username:    %s  %d\n", current->data.uname, current->data.uname_l);
    }
    if(current->data.pwd){
      printf("Password:    %s  %d\n", current->data.pwd, current->data.pwd_l);
    }
    current = current->next;
}

}
//memzeroes and frees every member of the list to avoid memory and data leaks
int free_list(node_t * head){
  if(!head){return 1;}
  node_t * current = head;
  node_t * next = current->next; 
  sodium_memzero(current->data.ename, current->data.ename_l);
  sodium_memzero(current->data.uname, current->data.uname_l);
  sodium_memzero(current->data.pwd, current->data.pwd_l); 
  if(current->data.ename && current->data.uname && current->data.pwd){
    free(current->data.ename);
    free(current->data.uname);
    free(current->data.pwd);
  }
  free(current);

  while(next){
    current = next;
    next = current->next;
    sodium_memzero(current->data.ename, current->data.ename_l);
    sodium_memzero(current->data.uname, current->data.uname_l);
    sodium_memzero(current->data.pwd, current->data.pwd_l); 
    if(current->data.ename && current->data.uname && current->data.pwd){
      free(current->data.ename);
      free(current->data.uname);
      free(current->data.pwd);
    }
    free(current);

  }

  return 0;
}

int edit_entry(node_t * head, char * entry_name, int option, char * new_value){
//very unreadable, beyond human comprehension even, will explain when finished
  node_t * current = head;
  int new_value_l = strlen(new_value);
  while( current ){
    if(strncmp(current->data.ename, entry_name, current->data.ename_l) == 0){
      switch(option){
      case 1:
        sodium_memzero(current->data.ename, current->data.ename_l);
	free(current->data.ename);
	current->data.ename = malloc(sizeof(char) * new_value_l);
	strncpy(current->data.ename, new_value, new_value_l);
	current->data.ename_l = new_value_l;
        return 0;
      case 2:
          sodium_memzero(current->data.uname, current->data.uname_l);
	  free(current->data.uname);
	  current->data.uname = malloc(sizeof(char) * new_value_l);
	  strncpy(current->data.uname, new_value, new_value_l);
	  current->data.uname_l = new_value_l;
          return 0;
      case 3:
          sodium_memzero(current->data.pwd, current->data.pwd_l);
	  free(current->data.pwd);
	  current->data.pwd = malloc(sizeof(char) * new_value_l);
	  strncpy(current->data.pwd, new_value, new_value_l);
	  current->data.pwd_l = new_value_l;
          return 0;
      }
    }
  current = current->next;
  }
  return 1;
}

int rem_spc(node_t ** head, char * entry_name){
//search the list for an entry and then remove that entry
  node_t * current = *head;
  if(strcmp(current->data.ename, entry_name) == 0){
    sodium_memzero(current->data.ename, current->data.ename_l);
    sodium_memzero(current->data.uname, current->data.uname_l);
    sodium_memzero(current->data.pwd, current->data.pwd_l); 
    if(current->data.ename && current->data.uname && current->data.pwd){
      free(current->data.ename);
      free(current->data.uname);
      free(current->data.pwd);
    }
    *head = current->next;
    free(current);
    return 0;
  }
  while( current->next ){
    if(strcmp(current->next->data.ename, entry_name) == 0){
      node_t * temp = current->next->next;
      sodium_memzero(current->next->data.ename, current->next->data.ename_l);
      sodium_memzero(current->next->data.uname, current->next->data.uname_l);
      sodium_memzero(current->next->data.pwd, current->next->data.pwd_l); 
      if(current->next->data.ename && current->next->data.uname && current->next->data.pwd){
        free(current->next->data.ename);
        free(current->next->data.uname);
        free(current->next->data.pwd);
      }
      current->next = temp;
      return 0;
    }
  current = current->next;
  }
  
  return 1;
}

int get_arg(char * str, char *** args){
//remove newlines before using
  char ** arg;
  int arg_count = 0;
  bool seq = false;
  int arg_len = 0;
  int arg_cur = 0;
//figure out how many args there are
  for(int i = 0; i < strlen(str); i++ ){
    if(str[i] != ' ' && seq == false){ ++arg_count; seq = true;}
    else if(str[i] == ' ' && seq == true){ seq = false; }
  }
//make room
  arg = malloc(sizeof(char*) * arg_count);
//figure out the length of each argument
  for(int i = 0; i < strlen(str); i++ ){
    if(str[i] != ' '){ arg_len++ ;}
//i barely understand how this works but it does so leave it
    if( i != 0 && str[i] == ' ' && str[i - 1] != ' ' || str[i+1] == '\0' && str[i] != ' '){
      arg[arg_cur] = calloc(1, sizeof(char) * arg_len);
      ++arg_cur;
      arg_len = 0;
    }
  }
//here copy the arguments into the space made previously, arg[0] will be the command used
  arg_len = 0;
  arg_cur = 0;
  for(int i = 0; i < strlen(str); i++ ){
    if(str[i] != ' '){ arg[arg_cur][arg_len] = str[i]; ++arg_len; }
    if(i != 0 && str[i] == ' ' && str[i - 1] != ' ' || str[i+1] == '\0' && str[i] != ' '){
      arg_len = 0;
      ++arg_cur;
    }
  }

  *args = arg;
  return arg_count;
}
