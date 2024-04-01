#define SALT_L 64


int save(node_t * head, unsigned char * name, unsigned char * password, unsigned char * path){
  if(!head){return 1;}
  int pass_len = strlen(password);
  //if(pass_len < 8){puts("A password length lower than 8 is not allowed");}
  unsigned char full_path[PATH_L];
  memset(full_path, '\0', PATH_L);
  strncat(full_path, path, PATH_L - 1);
  strncat(full_path, "/", PATH_L - 1);
  strncat(full_path, name, PATH_L - 1);
  strncat(full_path, ".mldb", PATH_L - 1);
  if(fexists(full_path) == 0){
  printf("The file %s already exists, are you sure the password is correct [n/y]? ", name);
  unsigned char ch = getchar();
  while(getchar() != '\n');
  if(ch != 'y' && ch != 'Y'){return 1;}
  }
  hash_state md;
  unsigned char iv[16];//init vec for enc
  unsigned char salt[SALT_L];
  unsigned char hash[32];//hash of salted password
  unsigned char key[32];//hash of password used as key
  unsigned char * salt_pass = calloc(1 ,sizeof(char) * (pass_len + SALT_L + 1));//salted password
  if(!salt_pass){return 1;}
  unsigned char * db = NULL;//decrypted database
  unsigned char * db_enc = NULL;//encrypted database
  unsigned int enc_len = 0;//length of encrypted text
  FILE * fp = fopen(full_path, "w");
  if(!fp){return 1;}
  node_t * current = head;
  unsigned int total_len = 0;
  sha256_init(&md);//generate key
  sha256_process(&md, password , pass_len);
  sha256_done(&md, key);
  //get total length
  while(current){
    total_len += current->data.ename_l;
    total_len += current->data.uname_l;
    total_len += current->data.pwd_l;
    total_len += 3;
    current = current->next;
  }
//generate salt
  for(int i = 0; i < 32; ++i){
    salt[i] = randombytes_uniform(256);
  }
//generate iv
  for(int i = 0; i < 16; ++i){
    iv[i] = randombytes_uniform(256);
  }
  fwrite(iv, 1, 16, fp);
//add salt to password
  strncpy(salt_pass, password, pass_len);
  for(int i = pass_len; i < pass_len + SALT_L; ++i){
    salt_pass[i] = salt[i - pass_len];
  }
//hash salted password
  sha256_init(&md);
  sha256_process(&md, salt_pass, pass_len + SALT_L);
  sha256_done(&md, hash);
  db = calloc(sizeof(char) * (total_len + SALT_L + 32) ,1);
  enc_len = round_up(total_len + SALT_L + 32, 16);
  db_enc = calloc(1, sizeof(char) * (enc_len) + 1);
  memset(db, '0', SALT_L + 32);
  current = head;
  //move entry values to db
  while(current){
    strncat(db, current->data.ename, current->data.ename_l);
    strcat(db, " ");
    strncat(db, current->data.uname, current->data.uname_l);
    strcat(db, " ");
    strncat(db, current->data.pwd, current->data.pwd_l);
    strcat(db, "\n");
    current = current->next;
  }
  //write salt and hash to database
  memcpy(db, salt, SALT_L);
  for(int i = SALT_L; i < SALT_L + 32; ++i){
    db[i] = hash[i - SALT_L];
  }
//encrypt and write to file
  if( encrypt(db, db_enc, key, iv, enc_len) == 0){
    fwrite(db_enc, 1, enc_len, fp);
    fflush(fp); 
  }
  else{puts("Error encrypting database");}
puts;
//memzero and free everything to avoid data leak
  sodium_memzero(salt, SALT_L);
  sodium_memzero(hash, 32);
  sodium_memzero(key, 32);

//possible that this first part has something to do with the load function
  if(salt_pass){
    sodium_memzero(salt_pass, pass_len + SALT_L);
    puts("1");
    free(salt_pass);//error here when saving with keyfile
    puts("2");
  }
  if(db){
    sodium_memzero(db, total_len + SALT_L + 32);
    free(db);
  }
  if(db_enc){
    puts("3");
    free(db_enc);//error here when saving normally
    puts("4");
  }
  //this bit of code being excluded causes a memleak, but i really cant figure out why it does a 
  //free(): invalid pointer sometimes, TODO:figure out later why this happens sometimes but not other times
  //works just fine without it but still have to fix it because it causes memory leak
  //this caused me to find another bug in the load function which made a crash if the file that
  //is being loaded from is empty, fixed that tho
  fclose(fp);
  return 0;
}


int load(node_t ** head, unsigned char * name, unsigned char * password, unsigned char * path){

  unsigned char full_path[PATH_L];
  memset(full_path, '\0', PATH_L);
  strncat(full_path, path, PATH_L - 1);
  strncat(full_path, "/", PATH_L - 1);
  strncat(full_path, name, PATH_L - 1);
  strncat(full_path, ".mldb", PATH_L - 1);
  if(fexists(full_path) != 0){
    puts("File not found");
    return 1;
  }
  unsigned char salt[SALT_L];
  hash_state md;
  unsigned int pass_len = strlen(password);
  unsigned char iv[16];//init vec for dec
  unsigned char comp_hash[32];//comparation hash read from file
  unsigned char salt_hash[32];//hash generated from password and salt from file
  unsigned char pwd_hash[32];//hash generated from password
  FILE * fp = fopen(full_path ,"r");
  if(fp){
    fseek(fp, 0, SEEK_END);
  }
  else{return 1;}
  int file_size = ftell(fp);
  if(file_size <= 0){puts("File can not be empty");return 1;}
  unsigned char * salt_pass = calloc(1, sizeof(unsigned char) * (pass_len + SALT_L));
  rewind(fp);
  fread(iv, 1, 16, fp);
  unsigned char * db = calloc(1, sizeof(unsigned char) * file_size - 16);//decrypted database
  unsigned char * db_enc = calloc(1, sizeof(unsigned char) * file_size - 16); //encrypted database
  fread(db_enc, 1, file_size - 16, fp);
  fclose(fp);
  //generate key
  sha256_init(&md);
  sha256_process(&md, password , pass_len);
  sha256_done(&md, pwd_hash);
  if(decrypt(db_enc, db, pwd_hash, iv, file_size - 16) != 0){
    sodium_memzero(pwd_hash , 32);
    sodium_memzero(salt_pass, pass_len + SALT_L);
    free(salt_pass);
    free(db);
    free(db_enc);
  }
  //load decrypted salt and hash from db
  memcpy(salt, db, SALT_L);
  for(int i = SALT_L; i < 32 + SALT_L; ++i){
    comp_hash[i - SALT_L] = db[i];
  }
  strncat(salt_pass, password, pass_len);
  for(int i = pass_len; i < pass_len + SALT_L; ++i){
    salt_pass[i] = salt[i - pass_len];
  }
//hash comparation password
  sha256_init(&md);
  sha256_process(&md, salt_pass, pass_len + SALT_L);
  sha256_done(&md, salt_hash);
//compare our hash and the hash from the file
  if(memcmp(comp_hash, salt_hash, 32) != 0){
    puts("Incorrect password");
    sodium_memzero(pwd_hash , 32);
    sodium_memzero(salt_pass, pass_len + SALT_L);
    free(salt_pass);
    free(db);
    free(db_enc);
    return 1;
  }
  free_list(*head);
  *head = EMPTY_LIST;
//load entries to head starting from pass_len + SALT_L
  int ename_l = 0;
  int uname_l = 0;
  int pwd_l = 0;
  int cur = SALT_L + 32;
  int current_part = 0;// 0 is entryname, 1 is username, 2 is passworddd
  int start;
  int total_entries = 0;
  for(int i = SALT_L + 32; i < file_size - 16; ++i){
    if(db[i] == '\n'){++total_entries;}
  }
  for(int i = 0; i < total_entries; ++i){
//start to first value
    start = cur;
//get length of all entry parts
    while(db[cur] != '\n'){
      if(db[cur] == ' '){current_part = (current_part + 1) % 3; ++cur; continue;}
      switch(current_part){
        case 0: ++ename_l; break;
        case 1: ++uname_l; break;
        case 2: ++pwd_l; break;
      }
    ++cur;
    }
//allocate space for those parts
    unsigned char * ename = calloc(1, sizeof(unsigned char) * (ename_l + 1));
    unsigned char * uname = calloc(1, sizeof(unsigned char) * (uname_l + 1));
    unsigned char * pwd = calloc(1, sizeof(unsigned char) * (pwd_l + 1));
//save current then copy everything starting from start
    int save = cur + 1;
    cur = start;
//Copy everything over
    for(int i = cur; i < cur + ename_l; ++i){
      ename[i - cur] = db[i];
    }
    cur += ename_l;
    cur += 1;
    for(int i = cur; i < cur + uname_l; ++i){
      uname[i - cur] = db[i];
    }
    cur += uname_l;
    cur += 1;
    for(int i = cur; i < cur + pwd_l; ++i){
      pwd[i - cur] = db[i];
    }
    cur = save;
//push new entry
    push(head, ename, uname, pwd, false);
//memzero and free everything
    sodium_memzero(ename, ename_l);
    sodium_memzero(uname, uname_l);
    sodium_memzero(pwd, pwd_l);
    free(ename);
    free(uname);
    free(pwd);
    ename_l = 0;
    uname_l = 0;
    pwd_l = 0;
    current_part = 0;
  }


  sodium_memzero(pwd_hash , 32);
  sodium_memzero(salt_pass, pass_len + SALT_L);
  free(salt_pass);
  sodium_memzero(db, file_size - 16);
  free(db);
  free(db_enc);
  sodium_memzero(comp_hash , 32);
  sodium_memzero(salt_hash , 32);
  sodium_memzero(pwd_hash , 32);
  sodium_memzero(salt , SALT_L);
  return 0;
}
