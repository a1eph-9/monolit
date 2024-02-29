#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>

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
  new_node->data.ename = malloc( sizeof(char) * new_node->data.ename_l);  
  new_node->data.uname = malloc( sizeof(char) * new_node->data.uname_l);  
  new_node->data.pwd = malloc(   sizeof(char) * new_node->data.pwd_l);  

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
      printf("Entryname:     %s  %d\n", current->data.ename, current->data.ename_l);
    }		
    if(current->data.uname){
      printf("Username:      %s  %d\n", current->data.uname, current->data.uname_l);
    }
    if(current->data.pwd){
      printf("Password:      %s  %d\n", current->data.pwd, current->data.pwd_l);
    }
    puts("\n");
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
    if(strcmp(current->data.ename, entry_name) == 0){
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

int main(int argc, char **argv){
  node_t * head = EMPTY_LIST;
  push(&head,"test","asdf","hgjk");
  push(&head,"test1","asdf","hgjk");
  push(&head,"test2","asdf","hgjk");
  print_list(head);
  int ret_val = edit_entry(head, "test2", 2, "sucess");
  if(ret_val != 0){puts("something went wrong");}

  ret_val = rem_spc(&head, "test1");
  if(ret_val != 0){puts("something went wrong");}

  puts("\n\nSecond print:\n");
  print_list(head);
  free_list(head);
  head = EMPTY_LIST;
  return 0;
}
