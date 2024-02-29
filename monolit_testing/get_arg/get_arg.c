#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

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
      arg[arg_cur] = malloc(sizeof(char) * arg_len);
      printf("%d\n", arg_len);
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
      printf("argument %d: ", arg_cur);
      printf("%s\n", arg[arg_cur]);
      arg_len = 0;
      ++arg_cur;
    }
  }

  *args = arg;
  return arg_count;
}

int main(int argc, char ** argv){
  char string[256];
  char ** args;
  fgets(string, 255, stdin); 
  string[strlen(string)-1] = '\0';
  int arg_count = get_arg(string, &args);
  printf("arg count: %d\n", arg_count);
}
