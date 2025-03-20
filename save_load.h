#define SALT_L 16
#define VER "monolit1.4" //the first 7 letters of this should not be changed



int load(node_t ** head, char * name, char * password, char * path){
  char * full_path = calloc(1, sizeof(char) * PATH_L);
  if(!full_path){return 1;}

  bool is_path = false;
  for(int i = 0; i < strlen(path); ++i){
    if(name[i] == '/'){is_path = true; break;}
  }
  if(is_path == false){
    strncat(full_path, path, PATH_L - 1);
    strncat(full_path, name, PATH_L - 1);
    strncat(full_path, ".mldb", PATH_L - 1);
  }
  else{
    strncpy(full_path, name, PATH_L - 1);
    strncat(full_path, ".mldb", PATH_L - 1);
  }
  if(fexists(full_path) != 0){
    puts("File not found");
    return 1;
  }
  char salt[SALT_L];
  hash_state md;
  unsigned int pass_len = strlen(password);
  char iv[16];//init vec for dec
  char comp_hash[32];//comparation hash read from file
  char salt_hash[32];//hash generated from password and salt from file
  char integ_hash[32];//hash to check integrity of file
  char comp_integ_hash[32];//hash to check integrity of file
  char pwd_hash[32];//hash generated from password
  char version[strlen(VER) + 1];
  memset(version, '\0', strlen(VER) + 1);
  FILE * fp = fopen(full_path ,"r");
  free(full_path);
  if(fp){
    fseek(fp, 0, SEEK_END);
  }
  else{return 1;}
  unsigned int file_size = ftell(fp) - strlen(VER);
  if(file_size % 16 != 0 || file_size + strlen(VER) < 112 + strlen(VER)){puts("Database has been corrupted or tampered with");fclose(fp);return 1;}
  char * salt_pass = calloc(1, sizeof(char) * (pass_len + SALT_L + 1));
  if(!salt_pass){fclose(fp);return 1;}
  rewind(fp);
  fread(version, 1, strlen(VER), fp);
  if(memcmp(version, "monolit", 7) != 0){
    puts("File is not a monolit database");
    free(salt_pass);
    fclose(fp);
    return 1;
  }
  if(memcmp(version, VER, strlen(VER)) != 0){
    puts("Incorrect database version");
    printf("Would you still like to open it?. This may cause a crash [y/n]? ");
    char ch = getchar();
    while(getchar() != '\n');
    if(ch != 'y' && ch != 'Y'){
      free(salt_pass);
      fclose(fp);
      return 1;
    }
  }
  fread(iv, 1, 16, fp);
  char * db = calloc(1, sizeof(char) * (file_size - 16 + 1));//decrypted database
  char * db_enc = calloc(1, sizeof(char) * (file_size - 16)); //encrypted database
  if(!db || !db_enc){
    free(salt_pass);
    fclose(fp);
    puts("Error allocating space");
    return 1;
  }
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
    return 1;
  }
  //load salt 
  memcpy(salt, db, SALT_L);
  //load password hash from db
  for(int i = SALT_L; i < 32 + SALT_L; ++i){
    comp_hash[i - SALT_L] = db[i];
  }
  memcpy(salt_pass, password, pass_len);
  for(int i = pass_len; i < pass_len + SALT_L; ++i){
    salt_pass[i] = salt[i - pass_len];
  }
//load comparation integrity hash from db
  for(int i = SALT_L + 32; i < SALT_L + 64 ; ++i){
    comp_integ_hash[i - SALT_L - 32] = db[i];
  }


//hash comparation password
  sha256_init(&md);
  sha256_process(&md, salt_pass, pass_len + SALT_L);
  sha256_done(&md, salt_hash);
//compare our hash and the hash from the file
  if(sodium_memcmp(comp_hash, salt_hash, 32) != 0){
    puts("Incorrect password");
    sodium_memzero(pwd_hash , 32);
    sodium_memzero(salt_pass, pass_len + SALT_L);
    sodium_memzero(db , file_size - 16);
    free(salt_pass);
    free(db);
    free(db_enc);
    return 1;
  }
//get the same total len as when saving
  unsigned int total_len = 0;
  for(int i = file_size - 16; i > SALT_L + 64; --i){
    if(db[i] == '\n'){ total_len = i - SALT_L - 64 + 1; break;}
  }
//generate integrity hash
  sha256_init(&md);
  sha256_process(&md, db + SALT_L + 64, total_len);
  sha256_done(&md, integ_hash);
//compare integrity hashes from file and generated from file
  if(sodium_memcmp(integ_hash, comp_integ_hash, 32) != 0){
    puts("Database has been corrupted or tampered with");
    printf("Would you still like to open it?. This may cause unexpected behaviour. [n/y] ");
    char ch = getchar();
    while(getchar() != '\n');
    if(ch != 'y' && ch != 'Y'){
      sodium_memzero(pwd_hash , 32);
      sodium_memzero(salt_hash , 32);
      sodium_memzero(comp_integ_hash , 32);
      sodium_memzero(integ_hash , 32);
      sodium_memzero(comp_hash , 32);
      sodium_memzero(salt_pass, pass_len + SALT_L);
      sodium_memzero(db, file_size - 16);
      free(salt_pass);
      free(db);
      free(db_enc);
      return 1;
    }
  }
  free_list(*head);
  *head = EMPTY_LIST;
//load entries to head starting from pass_len + SALT_L
  unsigned int ename_l = 0;
  unsigned int uname_l = 0;
  unsigned int pwd_l = 0;
  unsigned int cur = SALT_L + 64;
  unsigned int start;
  unsigned int total_entries = 0;
  for(int i = SALT_L + 32 + 32; i < file_size - 16; ++i){
    if(db[i] == '\n'){++total_entries;}
  }
  if(total_entries == 0){return 1;}
  total_entries = total_entries / 3;
  for(int i = 0; i < total_entries; ++i){
//start to first value
    start = cur;
//get length of all entry parts
    while (db[cur] != '\n' && cur < file_size - 16){
      ++ename_l;
      ++cur;
    }
    ++cur;
    while (db[cur] != '\n' && cur < file_size - 16){
      ++uname_l;
      ++cur;
    }
    ++cur;
    while (db[cur] != '\n' && cur < file_size - 16){
      ++pwd_l;
      ++cur;
    }
//allocate space for those parts
    char * ename = calloc(1, sizeof(char) * (ename_l + 1));
    char * uname = calloc(1, sizeof(char) * (uname_l + 1));
    char * pwd = calloc(1, sizeof(char) * (pwd_l + 1));
    if(!ename || !uname || !pwd){
      puts("Error allocating space");
      break;
    }
    //copy everything starting from start
   cur = start;
//Copy everything over
    for(int i = cur; i < cur + ename_l; ++i){
      ename[i - cur] = db[i];
    }
    cur += ename_l + 1;
    for(int i = cur; i < cur + uname_l; ++i){
      uname[i - cur] = db[i];
    }
    cur += uname_l + 1;
    for(int i = cur; i < cur + pwd_l; ++i){
      pwd[i - cur] = db[i];
    }
    cur += pwd_l + 1;
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
  sodium_memzero(comp_integ_hash , 32);
  sodium_memzero(integ_hash , 32);
  sodium_memzero(salt , SALT_L);
  return 0;
}



int verif_pass(char * pass, char * full_path){
  char iv[16];
  char * salt_pass;
  char db_enc[SALT_L + 64];
  char salt[SALT_L];
  char db[SALT_L + 64];
  char pwd_hash[32];
  char comp_hash[32];
  int pass_len = strlen(pass);
  FILE * fp = fopen(full_path ,"r");
  hash_state md;
  if(!fp){return 1;}
  fseek(fp, strlen(VER), SEEK_SET);
  fread(iv, 1, 16, fp);
  fread(db_enc, 1, SALT_L + 32, fp); 
  fclose(fp);
  sha256_init(&md);//generate key
  sha256_process(&md, pass , pass_len);
  sha256_done(&md, pwd_hash);
  if(decrypt(db_enc, db, pwd_hash, iv, SALT_L + 32) != 0){
    return 1;
  }
  memcpy(salt, db, SALT_L);
  memcpy(comp_hash, &(db[SALT_L]), 32);
  
  salt_pass = calloc(1, sizeof(char) * (pass_len + SALT_L + 1));
  memcpy(salt_pass, pass, pass_len);
  for(int i = pass_len; i < pass_len + SALT_L; ++i){
    salt_pass[i] = salt[i - pass_len];
  }
  sha256_init(&md);//generate comp hash
  sha256_process(&md, salt_pass , pass_len + SALT_L);
  sha256_done(&md, pwd_hash);
  free(salt_pass);
  if(memcmp(pwd_hash, comp_hash, 32) != 0){return 1;}
  sodium_memzero(pwd_hash, 32);
  sodium_memzero(comp_hash, 32);
  sodium_memzero(db, SALT_L + 64);
  return 0;
}



int save(node_t * head, char * name, char * password, char * path){
  if(!head){puts("No entries loaded");return 0;}
  int pass_len = strlen(password);
  if(pass_len < 8){puts("A password length lower than 8 is not allowed"); return 1;}
  char * full_path = calloc(1, sizeof(char) * (PATH_L + 1));
  if(!full_path){return 1;}
  bool is_path = false;
  for(int i = 0; i < strlen(path); ++i){
    if(name[i] == '/'){is_path = true; break;}
  }
  if(!is_path){
    strncat(full_path, path, PATH_L - 1);
    strncat(full_path, name, PATH_L - 1);
    strncat(full_path, ".mldb", PATH_L - 1);
  }
  else{
    strncpy(full_path, name, PATH_L - 1);
    strncat(full_path, ".mldb", PATH_L - 1);
  }
  if(fexists(full_path) == 0){
    if(verif_pass(password, full_path) != 0){
      puts("Incorrect password");
      printf("Would you like to save with the password \"%s\"? [n/y] ", password);
      char ch = getchar();
      if(ch != 'y' && ch != 'Y'){return 1;}
      while(getchar != '\n')
    }
  }
  hash_state md;
  char iv[16];//init vector for enc
  char salt[SALT_L];
  char hash[32];//hash of salted password
  char integ_hash[32];//hash of salted password
  char key[32];//hash of password used as key
  char * salt_pass = calloc(1 ,sizeof(char) * (pass_len + SALT_L + 1));//salted password
  if(!salt_pass){free(full_path);return 1;}
  char * db = NULL;//decrypted database
  char * db_enc = NULL;//encrypted database
  unsigned int enc_len = 0;//length of encrypted text
  FILE * fp = fopen(full_path, "w");
  free(full_path);	
  if(!fp){puts("Could not open file");free(salt_pass); return 1;}
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
  fwrite(VER, 1, strlen(VER), fp);
  fwrite(iv, 1, 16, fp);
//add salt to password
  memcpy(salt_pass, password, pass_len);
  for(int i = pass_len; i < pass_len + SALT_L; ++i){
    salt_pass[i] = salt[i - pass_len];
  }

//hash salted password
  sha256_init(&md);
  sha256_process(&md, salt_pass, pass_len + SALT_L);
  sha256_done(&md, hash);
  db = calloc(sizeof(char) * (total_len + SALT_L + 64 + 1) ,1);
  enc_len = round_up(total_len + SALT_L + 64, 16);
  db_enc = calloc(1, sizeof(char) * (enc_len));
  if(db == NULL || db_enc == NULL){
    fclose(fp); 
    free(salt_pass);
    puts("Error allocating space");
    return 1;
  }
  current = head;
  //move entry values to db
  unsigned int pos = 64 + SALT_L;
  while(current){
    memcpy(&(db[pos]), current->data.ename, current->data.ename_l);
    pos += current->data.ename_l;
    db[pos] = '\n'; ++pos;
    memcpy(&(db[pos]), current->data.uname, current->data.uname_l);
    pos += current->data.uname_l;
    db[pos] = '\n'; ++pos;
    memcpy(&(db[pos]), current->data.pwd, current->data.pwd_l);
    pos += current->data.pwd_l;
    db[pos] = '\n'; ++pos;

    current = current->next;
  }
//write salt
  memcpy(db, salt, SALT_L);
//write hash of password
  for(int i = SALT_L; i < SALT_L + 32; ++i){
    db[i] = hash[i - SALT_L];
  }
//generate integrity hash
  sha256_init(&md);
  sha256_process(&md, db + SALT_L + 64 , total_len);
  sha256_done(&md, integ_hash);
//write integrity hash to db
  for(int i = SALT_L + 32; i < SALT_L + 64; ++i){
    db[i] = integ_hash[i - SALT_L - 32];
  }

//encrypt and write to file
  if( encrypt(db, db_enc, key, iv, enc_len) == 0){
    fwrite(db_enc, 1, enc_len, fp);
    fflush(fp); 
  }
  else{puts("Error encrypting database");}

//memzero and free everything to avoid data leak
  sodium_memzero(salt, SALT_L);
  sodium_memzero(hash, 32);
  sodium_memzero(key, 32);
  if(salt_pass){
    sodium_memzero(salt_pass, pass_len + SALT_L);
    free(salt_pass);  
  }
  if(db){
    sodium_memzero(db, total_len + SALT_L + 64);
    free(db);
  }
  if(db_enc){
    free(db_enc);
  }
  fclose(fp);
  return 0;
}

