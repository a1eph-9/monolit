#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include <stdbool.h>
#include <unistd.h>

#define EMPTY_LIST NULL

#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define UPPER_L 26
#define LOWER "abcdefghijklmnopqrstuvwxyz"
#define LOWER_L 26
#define NUM "0123456789"
#define NUM_L 10
#define SPEC "!@#$%^&*()-_=+`~[]{}\\|;:'\",.<>/?"
#define SPEC_L 32
#define ARG_MAX 512
#define PATH_L ARG_MAX + 64

bool up = true;
bool low = true;
bool num = true;
bool spec = true;
bool help_msg = true;

typedef struct{
  char * ename;
  char * uname;
  char * pwd;
  unsigned int ename_l;
  unsigned int uname_l;
  unsigned int pwd_l;
}entry;

typedef struct node_t{
  entry data;
  struct node_t * next;
}node_t;

void print_logo(){
puts("\n   ███__  ███_   ██████_  ███_   ██_  ██████_  ██_      ██_ ████████_ ");
puts("   ████| ████|  ██____██_ ████_  ██| ██____██| ██|      ██| |__██___| ");
puts("   ██|████|██|  ██|   ██| ██|██_ ██| ██|   ██| ██|      ██|    ██| ");
puts("   ██||██_|██|  ██|   ██| ██||██_██| ██|   ██| ██|      ██|    ██| ");
puts("   ██| |_| ██|  |██████_| ██| |████| |██████_| ███████_ ██|    ██| ");
puts("   |_|     |_|   |_____|  |_|  |___|  |_____|  |______| |_|    |_| ");
puts("   A simple password manager written in C\n");
}

void wrong_arg(int ex, int go){
  if(go < ex){
    puts("Missing arguments");
  }
  else{puts("Too many arguments");}
}

int fexists(char *name){
  FILE *fp = fopen(name,"r");
  if(fp){
    fclose(fp);
    return 0;
  }
  return 1;
}

//self explanitory
int entry_exists(node_t * head, char * entry_name){
  if(!head){return 1;}
  node_t * current = head;
  while(current){
    if(strcmp(current->data.ename, entry_name) == 0){
      return 0;
    }
    current = current->next;
  }
  return 1;
}

int load_settings(char * path, char * last_db){
  char * full_path = calloc(1, sizeof(char) * (PATH_L + 1));
  if(!full_path){return 1;} 
  FILE * fp;
  uint8_t settings = 0;
  strncat(full_path, path, PATH_L - 1);
  strncat(full_path, "/config", PATH_L - 1);
  if(fexists(full_path) != 0){
   return 1; 
  }
  fp = fopen(full_path ,"r");
  free(full_path);
  if(!fp){return 1;}
  fseek(fp, 0, SEEK_END);
  unsigned int len = ftell(fp);
  if(len == 0){fclose(fp); return 1;}
  rewind(fp);
  char * conf = calloc(1, sizeof(char) * (len + 1));
  fread(conf, 1, len, fp);
//load what database was opened last
  int i = 0;
  while(i < len && conf[i] != ' '){
    last_db[i] = conf[i];
    ++i;
  }
//load settings
  if(i + 1 < len){
    settings = conf[i + 1];
    free(conf);
    if(settings % 2 != 1){ help_msg = false;}
    if((settings >> 1) % 2 != 1){ spec = false;}
    if((settings >> 2) % 2 != 1){ num = false;}
    if((settings >> 3) % 2 != 1){ low = false;}
    if((settings >> 4) % 2 != 1){ up = false;}
    fclose(fp);
    return 0;
}
  free(conf);
  return 1;
}

int save_settings(char * path, char * last_db){
  char * full_path = calloc(1, sizeof(char) * (PATH_L + 1));
  if(!full_path){return 1;}
  FILE * fp;
  uint8_t settings = 0;
  strncat(full_path, path, PATH_L - 1);
  strncat(full_path, "/config", PATH_L - 1);
  fp = fopen(full_path ,"w");
  free(full_path);
  if(!fp){return 1;}
//save last db
  fputs(last_db, fp); 
  fputc(' ', fp);
  if(up){++settings;}
  settings = settings << 1;
  if(low){++settings;}
  settings = settings << 1;
  if(num){++settings;}
  settings = settings << 1;
  if(spec){++settings;}
  settings = settings << 1;
  if(help_msg){++settings;}
  fputc(settings, fp);
  //save settings
  fclose(fp);
  return 0;
}

int toggle( char * opt){
  char set[] = "has been set to:";
  if(strcmp(opt, "show") == 0){
    printf("Upper: %s\n", up ? "true" : "false");
    printf("Lower: %s\n", up ? "true" : "false");
    printf("Number: %s\n", up ? "true" : "false");
    printf("Special: %s\n\n", up ? "true" : "false");
  }  
  else if(strcmp(opt, "upper") == 0){
    up = !up;
    printf("Upper %s %s\n", set, up ? "true" : "false");
  }

  else if(strcmp(opt, "lower") == 0){
    low = !low;
    printf("Lower %s %s\n", set, low ? "true" : "false");
  }
  else if(strcmp(opt, "number") == 0){
    num = !num;
    printf("Number %s %s\n", set, num ? "true" : "false");
  }
  else if(strcmp(opt, "special") == 0){
    spec = !spec;
    printf("Special %s %s\n ", set, spec ? "true" : "false");
  }
  else{
    printf("Invalid option: %s\n", opt);
    return 1;
  }
  return 0;
}


int round_up(int len, int block_size){
  if(len % block_size == 0){return len;}
  return len + (block_size - len % block_size);
}


char * pass_gen(int len){
  if(!up && !low && !num && !spec){
    puts("Must have at least one password generation option");
    return 0;
  }
  int available = 0;
  if(up){available += UPPER_L -1;}
  if(low){available += LOWER_L -1;}
  if(num){available += NUM_L -1;}
  if(spec){
    available += SPEC_L -1;
    available += SPEC_L - 1;
  }
  char * chars = calloc(1, sizeof(char) * (available + 1));
  if(!chars){return 0;}
  if(up){strncat(chars, UPPER, UPPER_L-1);}
  if(low){strncat(chars, LOWER, LOWER_L-1);}
  if(num){strncat(chars, NUM, NUM_L-1);}
  if(spec){
    strncat(chars, SPEC, SPEC_L-1);
    strncat(chars, SPEC, SPEC_L-1);
  }
  char * pass = calloc(1, sizeof(char) * (len + 1));
  if(!pass){free(chars); return 0;}
  for(int i = 0; i < len; ++i){
    pass[i] = chars[randombytes_uniform(available)];
  }
  free(chars);
  return pass;
}

int new_keyfile(char * name, char * len, char * path){
  char * full_path = calloc(1, sizeof(char) * (PATH_L + 1));
  if(!full_path){return 1;}
  strncat(full_path, path, PATH_L - 1);
  strncat(full_path, "/", PATH_L - 1);
  strncat(full_path, name, PATH_L - 1);
  strncat(full_path, ".mlkf", PATH_L - 1);
  int key_len = strtol(len, NULL, 10);
  if(key_len <= 0){puts("Lenght can not be less than 1"); return 1;}
  if(fexists(full_path) == 0){
    printf("The file %s already exists\n", name);
    return 1;
  }
  char * key = pass_gen(key_len);
  if(!key){free(full_path); return 1;}
  FILE * fp = fopen(full_path, "w");
  free(full_path);
  if(fp){
    fwrite("mlkf", 1, 4, fp);
    fwrite(key, 1, key_len, fp);
    sodium_memzero(key, key_len);
    free(key);
    fflush(fp);
    fclose(fp);
    return 0;
  }
  else if(fp){fclose(fp);}
  return 1;
}

char * load_keyfile(char * name, char * path){
  char * full_path = calloc(1, sizeof(char) * (PATH_L + 1));
  if(!full_path){return 0;}
  char verif[4];
  char * keyfile;
  strncat(full_path, path, PATH_L - 1);
  strncat(full_path, "/", PATH_L - 1);
  strncat(full_path, name, PATH_L - 1);
  strncat(full_path, ".mlkf", PATH_L - 1);
  if(fexists(full_path) != 0){
    puts("Keyfile was not found");
    return 0;
  }
  FILE * fp = fopen(full_path, "r");
  free(full_path);
  fseek(fp, 0, SEEK_END);
  unsigned int kf_len = ftell(fp);
  rewind(fp);
  if(kf_len <= 4){puts("Keyfile can not be shorter than 4 characters");fclose(fp);return 0;}
  fread(verif, 1, 4, fp);
  if(strncmp(verif, "mlkf", 4) != 0){puts("Invalid keyfile");fclose(fp); return 0;}
  keyfile = calloc(1, sizeof(char) * (kf_len - 3));
  if(keyfile){
    fread(keyfile, 1, kf_len - 4, fp);
  }
  fclose(fp);
  return keyfile;
}

int shred(char * name, char * path){
  char full_path[PATH_L];
  char command[] = {"shred -n 16 -z -u "};
  memset(full_path, '\0', PATH_L);
  strncat(full_path, path, PATH_L - 1);
  strncat(full_path, "/", PATH_L - 1);
  strncat(full_path, name, PATH_L - 1);
  strncat(full_path, ".mldb", PATH_L - 1);
  if(fexists(full_path) != 0){
    puts("File was not found");
    return 0;
  }
  printf("Are you sure %s is the correct database [n/y]? ", name);
  char ch = getchar();
  while(getchar() != '\n');
  if(ch != 'y' && ch != 'Y'){return 1;}
  system(strcat(command, full_path));
  return 0;
}

int push(node_t ** head, char * enm, char * unm, char * pwd, bool v) {
//setting up new node and transferring data
  unsigned int ename_l = strlen(enm);
  unsigned int uname_l = strlen(unm);
  unsigned int pwd_l = strlen(pwd);
  if(entry_exists(*head, enm) == 0){puts("Entry by the same name already exists");return 1;}
  node_t * new_node = malloc(sizeof(node_t));
  if(new_node){
    new_node->data.ename_l = ename_l;
    new_node->data.uname_l = uname_l;
    new_node->data.pwd_l = pwd_l;
  }
  else{return 1;}
  if(pwd_l < 8 && v == true){puts("A password length lower than 8 is not recommended");}
  new_node->data.ename = calloc(1 , sizeof(char) * (ename_l + 1));  
  new_node->data.uname = calloc(1, sizeof(char) * (uname_l + 1));  
  new_node->data.pwd = calloc(1, sizeof(char) * (pwd_l + 1));
  if(new_node->data.ename && new_node->data.uname && new_node->data.pwd){
    strncpy(new_node->data.ename, enm, ename_l);
    strncpy(new_node->data.uname, unm, uname_l);
    strncpy(new_node->data.pwd, pwd, pwd_l);
   //pushing new node to the list
  new_node->next = *head;
  *head = new_node;
  return 0;
  }
  free(new_node);
  return 1;
}


int print_list(node_t * head) {
//prints all entry values
  if(!head){puts("No entries loaded") ;return 1;}
  node_t * current = head;
  while (current) {
    printf("Entryname:   %s  %d\n", current->data.ename, current->data.ename_l);
    printf("Username:    %s  %d\n\n", current->data.uname, current->data.uname_l);
    current = current->next;
  }
  return 0;
}
//memzeroes and frees every member of the list to avoid memory and data leaks
int free_list(node_t * head){
  if(head == NULL){return 1;}
  node_t * current = head;
  node_t * next;
  while(current){
  sodium_memzero(current->data.ename, current->data.ename_l);
  sodium_memzero(current->data.uname, current->data.uname_l);
  sodium_memzero(current->data.pwd, current->data.pwd_l);
  free(current->data.ename);
  free(current->data.uname);
  free(current->data.pwd);
  next = current->next;
  free(current);
  current = next;
  }
  return 0;
}

int edit_entry(node_t * head, char * entry_name, char * option, char * new_value){
  node_t * current = head;
  uint8_t chosen = 0;
  unsigned int new_value_l = strlen(new_value);
  if(strncmp(option, "name", 4) == 0){chosen = 1;}
  else if(strncmp(option, "username", 8) == 0){chosen = 2;}
  else if(strncmp(option, "password", 8) == 0){chosen = 3;}
  else{puts("Invalid option"); return 1;}
  while( current ){
    if(strcmp(current->data.ename, entry_name) == 0){
      switch(chosen){
//edit entry name
      case 1:
        sodium_memzero(current->data.ename, current->data.ename_l);
	free(current->data.ename);
	current->data.ename = calloc(1, sizeof(char) * (new_value_l + 1));
	strncpy(current->data.ename, new_value, new_value_l);
	current->data.ename_l = new_value_l;
        return 0;
      case 2:
//edit username
          sodium_memzero(current->data.uname, current->data.uname_l);
	  free(current->data.uname);
	  current->data.uname = calloc(1, sizeof(char) * new_value_l);
	  strncpy(current->data.uname, new_value, (new_value_l + 1));
	  current->data.uname_l = new_value_l;
          return 0;
      case 3:
//edit password
          sodium_memzero(current->data.pwd, current->data.pwd_l);
	  free(current->data.pwd);
	  current->data.pwd = calloc(1, sizeof(char) * (new_value_l + 1));
	  strncpy(current->data.pwd, new_value, new_value_l);
	  current->data.pwd_l = new_value_l;
          return 0;
      }
    }
  current = current->next;
  }
  puts("Entry not found");
  return 1;
}

int rem_spc(node_t ** head, char * entry_name){
//search the list for an entry and then remove that entry
  if(*head == NULL){puts("No entries loaded");return 1;}
  node_t * current = *head;
  if(strcmp(current->data.ename, entry_name) == 0){
//only for first member of list
    sodium_memzero(current->data.ename, current->data.ename_l);
    sodium_memzero(current->data.uname, current->data.uname_l);
    sodium_memzero(current->data.pwd, current->data.pwd_l);
    free(current->data.ename);
    free(current->data.uname);
    free(current->data.pwd);
    *head = current->next;
    free(current);
    return 0;
  }
//for every next one
  while( current->next ){
    if(strcmp(current->next->data.ename, entry_name) == 0){
      node_t * temp = current->next->next;
      sodium_memzero(current->next->data.ename, current->next->data.ename_l);
      sodium_memzero(current->next->data.uname, current->next->data.uname_l);
      sodium_memzero(current->next->data.pwd, current->next->data.pwd_l);
      free(current->next->data.ename);
      free(current->next->data.uname);
      free(current->next->data.pwd);
      free(current->next);
      current->next = temp;
      return 0;
    }
  current = current->next;
  }
  puts("Entry was not found");
  return 1;
}

int get_arg(char * str, char *** args){
//remove newlines before using
  char ** arg;
  unsigned int arg_count = 0;
  bool seq = false;
  unsigned int arg_len = 0;
  unsigned int arg_cur = 0;
  unsigned int str_len = strlen(str);
//figure out how many args there are
  for(int i = 0; i < str_len; i++ ){
    if(str[i] != ' ' && seq == false){ ++arg_count; seq = true;}
    else if(str[i] == ' ' && seq == true){ seq = false; }
  }
//make room
  arg = calloc(1, sizeof(char *) * arg_count);
//figure out the length of each argument
  for(int i = 0; i < str_len; i++ ){
    if(str[i] != ' '){ arg_len++ ;}
//i barely understand how this works but it does so leave it
    if( (i != 0 && str[i] == ' ' && str[i - 1] != ' ') || (str[i+1] == '\0' && str[i] != ' ')){
      arg[arg_cur] = calloc(1, sizeof(char) * (arg_len + 1));
      ++arg_cur;
      arg_len = 0;
    }
  }
//here copy the arguments into the space made previously, arg[0] will be the command used
  arg_len = 0;
  arg_cur = 0;
  for(int i = 0; i < str_len; i++ ){
    if(str[i] != ' '){ arg[arg_cur][arg_len] = str[i]; ++arg_len; }
    if((i != 0 && str[i] == ' ' && str[i - 1] != ' ') || (str[i+1] == '\0' && str[i] != ' ')){
      arg_len = 0;
      ++arg_cur;
    }
  }

  *args = arg;
  return arg_count;
}



int print_spc(node_t * head, char * enm){
  node_t * current = head;
  while(current){
    if(strcmp(enm, current->data.ename) == 0){
      printf("Entryname:   %s  %d\n", current->data.ename, current->data.ename_l);
      printf("Username:    %s  %d\n", current->data.uname, current->data.uname_l);
      printf("Password:    %s  %d\n", current->data.pwd, current->data.pwd_l);
      return 0;
    }
    current = current->next;
  }
  puts("Entry was not found");
  return 1;
}



int encrypt(char plain_text[], char chiper_text[], char key[] , char iv[], unsigned int ciphertext_length) {
    symmetric_CBC cbc;
    int err;
    err = register_cipher(&aes_desc);
    if (err != CRYPT_OK) {
        printf("Error registering cipher: %s\n", error_to_string(err));
        return 1;
    }
    symmetric_key skey;
    err = aes_setup(key, 32, 0, &skey);
    if (err != CRYPT_OK) {
        printf("Error aes setup: %s\n", error_to_string(err));
        return 1;
    }
    err = cbc_start(find_cipher("aes"), iv, key, 32, 0, &cbc);
    if (err != CRYPT_OK) {
        printf("Error cbc start: %s\n", error_to_string(err));
        return 1;
    }
    err = cbc_encrypt(plain_text, chiper_text, ciphertext_length, &cbc);
    if (err != CRYPT_OK) {
        printf("Error cbc encrypt: %s\n", error_to_string(err));
        return 1;
    }
    err = cbc_done(&cbc);
    if (err != CRYPT_OK) {
        printf("Error cbc done: %s\n", error_to_string(err));
        return 1;
    }
    err = unregister_cipher(&aes_desc);
    if (err != CRYPT_OK) {
        printf("Error unregister cipher: %s\n", error_to_string(err));
        return 1;
    }
    return 0;
}



int decrypt(char chiper_text[], char plain_text[], char key[], char iv[], unsigned int chipertext_length) {
    symmetric_CBC cbc;
    int err;
    err = register_cipher(&aes_desc);
    if (err != CRYPT_OK) {
        printf("Error registering cipher: %s\n", error_to_string(err));
        return 1;
    }
    symmetric_key skey;
    err = aes_setup(key, 32, 0, &skey);
    if (err != CRYPT_OK) {
        printf("Error aes setup: %s\n", error_to_string(err));
        return 1;
    }
    err = cbc_start(find_cipher("aes"), iv, key, 32, 0, &cbc);
    if (err != CRYPT_OK) {
        printf("Error cbc start: %s\n", error_to_string(err));
        return 1;
    }
    err = cbc_decrypt(chiper_text, plain_text, chipertext_length, &cbc);
    if (err != CRYPT_OK) {
        printf("Error cbc decrypt: %s\n", error_to_string(err));
        return 1;
    }
    err = cbc_done(&cbc);
    if (err != CRYPT_OK) {
        printf("Error cbc done: %s\n", error_to_string(err));
        return 1;
    }
    err = unregister_cipher(&aes_desc);
    if (err != CRYPT_OK) {
        printf("Error unregister cipher: %s\n", error_to_string(err));
        return 1;
    }
    return 0;
}


int help(char * opt){
  if(strcmp(opt, "0") == 0){
    puts("How to find the details of any command:");
    puts("help *command name*");
    puts("To see all command names, and what they do:");
    puts("help all");
    return 0;
  }
  if(strcmp(opt, "all") == 0){
    puts("exit - close the program || no args");
    puts("clear - clear the screen || no args");
    puts("show - show a specific entry || 1 arg");
    puts("show_all - show all entries || no args");
    puts("new - make a new entry || 3 args");
    puts("new_r - make a new entry with a random password || 3 args");
    puts("r_pass - generate a random password || 1 arg");
    puts("remove - remove a specific entry || 1 arg");
    puts("edit - edit an entry || 3 args");
    puts("new_kf - create a new keyfile || 2 args");
    puts("save - save entries to file || 2 args");
    puts("load - load entries from file || 2 args");
    puts("toggle - toggle password generation options || 1 arg");
    puts("load_last - load last used database || 1 arg");
    puts("save_kf - save entries to file using keyfile|| 3 args");
    puts("load_kf - load entries from file using keyfile || 3 args");
    puts("shred - securely delete a database || 1 arg");
    return 0;
  }
  if(strcmp(opt, "show") == 0){ 
    puts("show - show a specific entry, args: ");
    puts("1: entry name");
    return 0;
  }

  if(strcmp(opt, "remove") == 0){ 
    puts("remove - remove a specific entry, args: ");
    puts("1: entry name");
    return 0;
  }

  if(strcmp(opt, "new") == 0){ 
    puts("new - make a new entry, args: ");
  }

  if(strcmp(opt, "show") == 0){ 
    puts("show - show a specific entry, args: ");
    puts("1: entry name");
    return 0;
  }

  if(strcmp(opt, "remove") == 0){ 
    puts("remove - remove a specific entry, args: ");
    puts("1: entry name");
    return 0;
  }

  if(strcmp(opt, "new") == 0){ 
    puts("new - make a new entry, args: ");
    puts("1: entry name");
    puts("2: username");
    puts("3: password");
    return 0;
  }
  if(strcmp(opt, "new_r") == 0){ 
    puts("new - make a new entry with a randomly generated password, args: ");
    puts("1: entry name");
    puts("2: username");
    puts("3: password length");
    return 0;
  }
   if(strcmp(opt, "r_pass") == 0){
     puts("r_pass - generate a random password, args: ");
     puts("1: password length");
     return 0;
   }

  if(strcmp(opt, "edit") == 0){ 
    puts("edit - edit a entry, args: ");
    puts("1: entry name");
    puts("2: option | this can be: name, username or password");
    puts("3: new value");
    return 0;
  }
   if(strcmp(opt, "save") == 0){ 
     puts("save - save entries to a database, args: ");
     puts("1: database name");
     puts("2: password");
     return 0;
   }
   if(strcmp(opt, "save_kf") == 0){ 
     puts("save_kf - save entries to a database using keyfile, args: ");
     puts("1: database name");
     puts("2: password");
     puts("3: keyfile name");
     return 0;
   }

   if(strcmp(opt, "load_kf") == 0){ 
     puts("load_kf - load entries from a database using keyfile, args: ");
     puts("1: database name");
     puts("2: password");
     puts("3: keyfile name");
     return 0;
  }
   if(strcmp(opt, "load") == 0){ 
     puts("load - load entries from a database, args: ");
     puts("1: database name");
     puts("2: password");
     return 0;
  }
   if(strcmp(opt, "load_last") == 0){ 
    puts("load_last - load last used database, args:");
     puts("1: password");
     return 0;
  }

   if(strcmp(opt, "new_kf") == 0){ 
     puts("new_kf - create a new keyfile, args: ");
     puts("1: keyfile name");
     puts("2: keyfile length");
     return 0;
  }  
   if(strcmp(opt, "shred") == 0){ 
     puts("shred - securely delete a database, args:");
     puts("1: database name");
     return 0;
  }

   if(strcmp(opt, "toggle") == 0){ 
     puts("toggle - toggle password generation options, args:");
     puts("1: Option name (can be: upper, lower, number or special)");
     return 0;
  }
   if(strcmp(opt, "off") == 0){
     help_msg = false;
     system("clear");
     print_logo();
     return 0;
  }
  printf("Unknown command: %s\n", opt);

  return 1;
}



