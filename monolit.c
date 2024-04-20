#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sodium.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tomcrypt.h>
#include "util.h"
#include "save_load.h"

struct stat st = {0};

int main(int argc, char **argv){
  if (sodium_init() < 0) {
    puts("Could not initiate libsodium");
    return 1;
  }
  unsigned char path[PATH_L];
  unsigned char arg_input[ARG_MAX];
  unsigned char **args;
  node_t * head = EMPTY_LIST;
  unsigned char * keyfile;
  int key_password;
  unsigned char * key_pass;
  unsigned char last_db[ARG_MAX];
  memset(path, '\0', ARG_MAX - 1);
  unsigned char name[64];
  getlogin_r(name, 64);
  memset(path, '\0', PATH_L - 1);
  strncat(path, "/home/", PATH_L - 1);
  strncat(path, name, PATH_L - 1);
  strncat(path, "/.monolit", PATH_L - 1);
  if (stat(path, &st) == -1) {
    mkdir(path, 0700);
  }
  load_settings(path, last_db);
  system("clear");
  print_logo();
  if(help_msg == true){puts("Use \"help\" to introduce yourself, to turn this message off do \"help off\"");}
  while(1){
    printf(">> ");
    fgets( arg_input ,ARG_MAX-1 , stdin);
    arg_input[strlen(arg_input) - 1] = '\0';
    unsigned int arg_count = get_arg(arg_input, &args);
    if(arg_count != 0){
      if(strcmp(args[0], "exit" ) == 0 ){
        puts("Exiting...");
        save_settings(path, last_db);
	free_list(head);
        for(int i = 0; i < arg_count; ++i){
          sodium_memzero(args[i], strlen(args[i]));
          if(args[i]){free(args[i]);}
        }
	sodium_memzero(arg_input, ARG_MAX);
        break;
      }
      else if(strcmp(args[0], "show_all" ) == 0 ){
        print_list(head);
      }

      else if(strcmp(args[0], "new" ) == 0 ){
        if(arg_count == 4 ){
          push(&head, args[1], args[2], args[3], true);;
	}
	else{wrong_arg(4, arg_count);}
	
      }

      else if(strcmp(args[0], "new_r" ) == 0 ){
        if(arg_count == 4 ){
	  int pass_len = strtol(args[3], NULL, 10);
	  if(pass_len > 0){
	  unsigned char * pass = pass_gen(pass_len);
          push(&head, args[1], args[2], pass, true);
	  sodium_memzero(pass, pass_len);
	  free(pass);
	  }
	  else{puts("Length can not be less than 1");}
	}
	else{wrong_arg(4, arg_count);}
      }

      else if(strcmp(args[0], "remove" ) == 0 ){
        if(arg_count == 2 ){
          rem_spc(&head, args[1]);
	}
	else{wrong_arg(2, arg_count);}
      }

      else if(strcmp(args[0], "show" ) == 0 ){
        if(arg_count == 2 ){
          print_spc(head, args[1]);
	}
	else{wrong_arg(2, arg_count);}
      }
      
      else if(strcmp(args[0], "clear" ) == 0 || strcmp(args[0], "clr") == 0){
          system("clear");
	  print_logo();
      }
      else if(strcmp(args[0], "save" ) == 0 ){
        if(arg_count == 3){
	  if(save(head, args[1] , args[2], path) == 0){
	    strncpy(last_db, args[1], ARG_MAX );
	  }
	}
	else{wrong_arg(3, arg_count);}
    }

    else if(strcmp(args[0], "load_last" ) == 0 ){
        if(arg_count == 2){
	  
	  if(load(&head, last_db , args[1], path) == 0){
            print_list(head);
	  }
	}
	else{wrong_arg(2, arg_count);}
    }

    else if(strcmp(args[0], "load" ) == 0 ){
        if(arg_count == 3){
	  if(load(&head, args[1] , args[2], path) == 0){
            print_list(head);
	    strncpy(last_db, args[1], ARG_MAX );
	  }
	}
	else{wrong_arg(3, arg_count);}
    }

    else if(strcmp(args[0], "save_kf" ) == 0 ){
        if(arg_count == 4){
	  keyfile = load_keyfile(args[3], path);
	  if(keyfile){
	    key_password = strlen(args[2]) + strlen(keyfile);
	    key_pass = calloc(1, sizeof(unsigned char) * (key_password + 1));
	    strncpy(key_pass, keyfile, key_password);
	    strncat(key_pass, args[2], key_password);
	    if(save(head, args[1], key_pass, path)){ 
	      strncpy(last_db, args[1], ARG_MAX );
	    }
	    sodium_memzero(key_pass, key_password);
	    free(keyfile);
	    free(key_pass);
	    keyfile = NULL;
	    key_pass = NULL;
	  }
	}
	else{wrong_arg(3, arg_count);}
    }

    else if(strcmp(args[0], "load_kf" ) == 0 ){
        if(arg_count == 4){
	  keyfile = load_keyfile(args[3], path);
	  if(keyfile){
	    key_password = strlen(args[2]) + strlen(keyfile);
	    key_pass = calloc(1, sizeof(unsigned char) * (key_password + 1));
	    strncpy(key_pass, keyfile, key_password);
	    strncat(key_pass, args[2], key_password);
	    if(load(&head, args[1], key_pass, path) == 0){
	      print_list(head);
	      strncpy(last_db, args[1], ARG_MAX );
	    }
	    sodium_memzero(key_pass, key_password);
	    free(keyfile);
	    free(key_pass);
	    keyfile = NULL;
	    key_pass = NULL;
	  }
	}
	else{wrong_arg(3, arg_count);}
    }


    else if(strcmp(args[0], "edit" ) == 0 ){
        if(arg_count == 4){
	  edit_entry(head, args[1] , args[2], args[3]);
	}
	else{wrong_arg(4, arg_count);}
    }


    else if(strcmp(args[0], "new_kf" ) == 0 ){

        if(arg_count == 3){
	  new_keyfile(args[1], args[2], path);
	}
	else{wrong_arg(3, arg_count);}
    }

    else if(strcmp(args[0], "help" ) == 0 ){
      if(arg_count == 2){
	    help(args[1]);
	  }
	  else if(arg_count == 1 ){
        help("0");//arbitrary character
	  }
	  else{wrong_arg(2, arg_count);}
    }

    else if(strcmp(args[0], "r_pass" ) == 0 ){
        if(arg_count == 2){
	  int rand_l = strtol(args[1], NULL, 10);
	  if(rand_l > 0){
	    unsigned char * rand = pass_gen(rand_l);
	    printf("Random password: %s\n", rand);
	    sodium_memzero(rand, rand_l);
	    free(rand);
	  }
	  else{puts("Length can not be less than 1");}

	}
	else{wrong_arg(2, arg_count);}
    }

    else if(strcmp(args[0], "shred" ) == 0 ){
        if(arg_count == 2){
	    shred(args[1], path);
	}
	else{wrong_arg(2, arg_count);}
    }

    else{printf("Unknown command: %s\n", args[0]);}
    
    sodium_memzero(arg_input, sizeof(char) * ARG_MAX);
    for(int i = 0; i < arg_count; ++i){
      sodium_memzero(args[i], strlen(args[i]));
      if(args[i]){free(args[i]);}
    }
    free(args);
    }
  }
  return 0;
}
