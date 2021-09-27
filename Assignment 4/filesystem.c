#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


/*
 *   ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ 
 *  |   |   |   |   |                       |   |
 *  | 0 | 1 | 2 | 3 |     .....             |127|
 *  |___|___|___|___|_______________________|___|
 *  |   \    <-----  data blocks ------>
 *  |     \
 *  |       \
 *  |         \
 *  |           \
 *  |             \
 *  |               \
 *  |                 \
 *  |                   \
 *  |                     \
 *  |                       \
 *  |                         \
 *  |                           \
 *  |                             \
 *  |                               \
 *  |                                 \
 *  |                                   \
 *  |                                     \
 *  |                                       \
 *  |                                         \
 *  |                                           \
 *  |     <--- super block --->                   \
 *  |______________________________________________|
 *  |               |      |      |        |       |
 *  |        free   |      |      |        |       |
 *  |       block   |inode0|inode1|   .... |inode15|
 *  |        list   |      |      |        |       |
 *  |_______________|______|______|________|_______|
 *
 *
 */


#define FILENAME_MAXLEN 8  // including the NULL char

/* 
 * inode 
 */

typedef struct inode {
  int  dir;  // boolean value. 1 if it's a directory.
  char name[FILENAME_MAXLEN];
  int  size;  // actual file/directory size in bytes.
  int  blockptrs [8];  // direct pointers to blocks containing file's content.
  int  used;  // boolean value. 1 if the entry is in use.
  int  rsvd;  // reserved for future use
} inode;


/* 
 * directory entry
 */

typedef struct dirent {
  char name[FILENAME_MAXLEN];
  int  namelen;  // length of entry name
  int  inode;  // this entry inode index
} dirent;

/*
 * functions
 */
// create file
int CR(char path[50],int size) {
  FILE* myfs = fopen("myfs", "r+"); // Opening myfs
  fseek(myfs,128 + 56,SEEK_SET);

  char actual_path[50];
  strcpy(actual_path,path);
  char *p = strtok(path,"/");
  char *name = NULL;
  while (p != NULL) { // Getting name of file that we have to create
    name = p;
    p = strtok(NULL,"/");
  }
  
  char *q = strtok(actual_path,"/");
  int check = 0;
  
  while (q!=NULL) { // Checking if any of the directory in the path does not exist
    rewind(myfs);
    fseek(myfs,128 + 56,SEEK_SET);
    check = 0;
    for (int i = 1; i < 16; i++) {
      inode node;
		  fread(&node, sizeof(inode), 1, myfs);
      if(strcmp(node.name,q) == 0) {
        check = 1;
      }
    }
    if (check == 0 && strcmp(q,name) != 0) {
      printf("the directory %s in the given path does not exist\n",q);
      return 1;
    }
    
    q = strtok(NULL,"/");
  }
  int j;
  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Checking if the file that we have to create is already there
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(strcmp(node.name,name) == 0) {
      check = 1;
    }
  }
  if (check == 1) {
    printf("%s\n","the file already exists");
    return 1;
  }
  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Finding empty inode
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(node.used == 0) {
      j = i;
      check = 1;
      break;
    }
  }
  if (check == 0) { // If empty node not found then output error
    printf("%s","not enough space\n");
    return 1;
  }
  
  int blocks_req = (size/1024) + (size % 1024 != 0);
  if (blocks_req > 8*1024) { // Checking if we can fulfil blocks requirement
    printf("%s","not enough space\n");
    return 1;
  }
  
  inode new;
  rewind(myfs);
  int freelist[128];
  fread(freelist, 128, 1, myfs);
  int block_num = 1;
  int blocks_gotten = 0;
	for (int i=0; i<blocks_req; i++) {
		while(block_num <= 1024) {
			if (freelist[block_num] != 1) { // Finding empty blocks where we can write data 
        new.blockptrs[i] = block_num;
        blocks_gotten++;
        if (block_num == blocks_req) {break;}
      }
			block_num++;
		}
    if (blocks_gotten > blocks_req) { // If empty block not found then output error
		  printf("error: not enough space\n");
		  return 1;
    }
	}

  new.dir = 0;
  new.size = size;
  new.used = 1;
  strcpy(new.name, name);
  new.rsvd = 0;
  
  rewind(myfs);
  fseek(myfs,128 + (j*56),SEEK_SET);
  fwrite(&new, sizeof(inode),1, myfs); // Writing inode with the required information

  char set = 1;
  char data[1024];
	int data_size = 1024;
  rewind(myfs);
	for (int i=0; i < blocks_req; i++) {
    rewind(myfs);
		fseek(myfs, new.blockptrs[i], SEEK_SET);
    fwrite(&set, sizeof(char), 1, myfs); //Writing free block list

		if (size>1024) {
      size-=1024;
    }
		else {
      data_size = size;
    }
		for (int k=0; k<data_size; k++) {
			data[k] = (char) (97 + ((k + i * 1024) % 26));
    }
    rewind(myfs);
		fseek(myfs, 1024 * new.blockptrs[i], SEEK_SET);
    fwrite(&data, sizeof(data_size), 1, myfs); // Writing Data in data block
	}
  fclose(myfs); // Closing file
  return 0;
}
// move a file 
int MV(char path[50],char to_path[50]) {
  FILE* myfs = fopen("myfs", "r+"); // Opening myfs
  fseek(myfs,128 + 56,SEEK_SET);

  char *p = strtok(path,"/");
  char *name = NULL;
  while (p != NULL) { // Getting name of file that we have to move
    name = p;
    p = strtok(NULL,"/");
  }
  char *q = strtok(path,"/");
  
  int check = 0;
  while (q!=NULL) { // Checking if any of the directory in the path does not exist
    rewind(myfs);
    fseek(myfs,128 + 56,SEEK_SET);
    check = 0;
    for (int i = 1; i < 16; i++) {
      inode node;
		  fread(&node, sizeof(inode), 1, myfs);
      if(strcmp(node.name,q) == 0) {
        check = 1;
      }
    }
    if (check == 0 && strcmp(q,name) != 0) {
      printf("the directory %s in the given path does not exist\n",q);
      return 1;
    }
    q = strtok(NULL,"/");
  }
  
  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Checking if the file that we have to move is not there
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(strcmp(node.name,name) == 0) {
      check = 1;
    }
  }
  if (check == 0) { // output error in case file does not exist
    printf("%s\n","this file does not exist");
    return 1;
  }

  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Checking if the path is to a directory
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(node.dir == 1 && strcmp(node.name,name) == 0) {
      check = 1;
    }
  }
  if (check == 1) { 
    printf("%s\n","can't handle directories");
    return 1;
  }
  inode original;
  int j;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Finding inode of source
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(strcmp(node.name,name) == 0) {
      original = node;
      j = i;
      break;
    }
  }
  original.used = 0; // Marking that node not used. not erasing data since it will be overwritten if need be
  strcpy(original.name,"");
  rewind(myfs);
  fseek(myfs,128 + j*56,SEEK_SET);
  fwrite(&original, sizeof(inode),1, myfs);
  CR(to_path,original.size); // Creating destination file with size of source file

  fclose(myfs); // Closing myfs
  return 0;
}
// remove/delete file
int DL(char path[50]) {
  FILE* myfs = fopen("myfs", "r+"); // Opening myfs
  fseek(myfs,128 + 56,SEEK_SET);

  char *p = strtok(path,"/");
  char *name = NULL;
  while (p != NULL) {
    name = p;
    p = strtok(NULL,"/");
  }
  char *q = strtok(path,"/");
  int check = 0;
  while (q!=NULL) { // Checking if any of the directory in the path does not exist
    rewind(myfs);
    fseek(myfs,128 + 56,SEEK_SET);
    check = 0;
    for (int i = 1; i < 16; i++) {
      inode node;
		  fread(&node, sizeof(inode), 1, myfs);
      if(strcmp(node.name,q) == 0) {
        check = 1;
      }
    }
    if (check == 0 && strcmp(q,name) != 0) {
      printf("the directory %s in the given path does not exist\n",q);
      return 1;
    }
    q = strtok(NULL,"/");
  }
  int j;
  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Checking if the file that we have to delete already is not there
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(strcmp(node.name,name) == 0) {
      check = 1;
      j = i;
    }
  }
  if (check == 0) {
    printf("%s\n","this file does not exist");
    return 1;
  }
  // not deleting data since it will be overwritten as we have declared this node not used
  inode new;
  new.used = 0;
  strcpy(new.name, "");
  rewind(myfs);
  fseek(myfs,128 + (j*56),SEEK_SET);
  fwrite(&new, sizeof(inode), 1, myfs);
  fclose(myfs);// Closing myfs
  return 0;
}
// copy a file
int CP(char path[20],char to_path[20]) {
  FILE* myfs = fopen("myfs", "r+"); // Opening myfs
  fseek(myfs,128 + 56,SEEK_SET);

  char *p = strtok(path,"/");
  char *name = NULL;
  while (p != NULL) {
    name = p;
    p = strtok(NULL,"/");
  }
  char *q = strtok(path,"/");
  
  int check = 0;
  while (q!=NULL) { // Checking if any of the directory in the path does not exist
    rewind(myfs);
    fseek(myfs,128 + 56,SEEK_SET);
    check = 0;
    for (int i = 1; i < 16; i++) {
      inode node;
		  fread(&node, sizeof(inode), 1, myfs);
      if(strcmp(node.name,q) == 0) {
        check = 1;
      }
    }
    if (check == 0 && strcmp(q,name) != 0) {
      printf("the directory %s in the given path does not exist\n",q);
      return 1;
    }
    q = strtok(NULL,"/");
  }
  
  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Checking if the file that we have to copy is not there
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(strcmp(node.name,name) == 0) {
      check = 1;
    }
  }
  if (check == 0) {
    printf("%s\n","this file does not exist");
    return 1;
  }

  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Checking if the path is to a directory
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(node.dir == 1 && strcmp(node.name,name) == 0) {
      check = 1;
    }
  }
  if (check == 1) {
    printf("%s\n","can't handle directories");
    return 1;
  }
  inode original;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Finding inode of source
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(strcmp(node.name,name) == 0) {
      original = node;
      break;
    }
  }
  CR(to_path,original.size); // Creating the destination file with the size of source file

  fclose(myfs);
  return 0;
}
// list file info
void LL() {
  FILE* myfs = fopen("myfs", "r+");	// Opening myfs
  fseek(myfs,128,SEEK_SET);

	inode node;
  printf("%s","Listing down the files/directories:\n");
	for (int i = 0; i < 16; i++) { // Iterating over all the inodes
		fread(&node, sizeof(inode), 1, myfs);
		if (node.used == 1 && node.dir == 1) { // If inode is a directory printing this
			printf("Directory: %s ", node.name);
			printf("%d\n", node.size);
		}
    else if (node.used == 1 && node.dir == 0) { // If inode is a file printing this
			printf("File: %s ", node.name);
			printf("%d\n", node.size);
		}
	}
  fclose(myfs); // Closing myfs
}
// create directory
int CD(char path[50]) {
  FILE* myfs = fopen("myfs", "r+"); // Opening myfs
  fseek(myfs,128 + 56,SEEK_SET);
  char actual_path[50];
  strcpy(actual_path,path);
  char *p = strtok(path,"/");
  char *name = NULL;
  while (p != NULL) {
    name = p;
    p = strtok(NULL,"/");
  }
  char *q = strtok(actual_path,"/");
  int check = 0;
  
  while (q!=NULL) { // Checking if any of the directory in the path does not exist
    rewind(myfs);
    fseek(myfs,128 + 56,SEEK_SET);
    check = 0;
    for (int i = 1; i < 16; i++) {
      inode node;
		  fread(&node, sizeof(inode), 1, myfs);
      if(strcmp(node.name,q) == 0) {
        check = 1;
      }
    }
    if (check == 0 && strcmp(q,name) != 0) {
      printf("the directory %s in the given path does not exist\n",q);
      return 1;
    }
    q = strtok(NULL,"/");
  }
  int j;
  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Checking if the directory that we have to create is already there
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(strcmp(node.name,name) == 0) {
      check = 1;
    }
  }
  if (check == 1) {
    printf("%s\n","the directory already exists");
    return 1;
  }
  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Finding empty inode
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(node.used == 0) {
      j = i;
      check = 1;
      break;
    }
  }
  if (check == 0) {
    printf("%s","not enough space\n");
    return 1;
  }
  int blocks_req = 1;
  if (blocks_req > 8*1024) { // Checking if we can fulfil blocks requirement
    printf("%s","not enough space\n");
    return 1;
  }
  inode new;
  rewind(myfs);
  int freelist[128];
  fread(freelist, 128, 1, myfs);
  int block_num = 1;
  int blocks_gotten = 0;
	for (int i = 0; i < blocks_req; i++) {
		while(block_num <= 1024) {
			if (freelist[block_num] != 1) {
        new.blockptrs[i] = block_num;
        blocks_gotten++;
        if (block_num == blocks_req) {break;}
      }
			block_num++;
		}
    if (blocks_gotten > blocks_req) {
		  printf("error: not enough space\n");
		  return 1;
    }
	}

  new.dir = 1;
  new.size = sizeof(dirent);
  new.used = 1;
  strcpy(new.name, name);
  new.rsvd = 0;
  
  rewind(myfs);
  fseek(myfs,128 + (j*56),SEEK_SET); // Writing the inode with the appropriate values
  fwrite(&new, sizeof(inode),1, myfs);
  dirent dir_node;
  strcpy(dir_node.name,name);
  dir_node.namelen = strlen(dir_node.name);
  dir_node.inode = j;
  rewind(myfs);
  fseek(myfs, 1024 * new.blockptrs[0], SEEK_SET);
  fwrite(&dir_node, sizeof(dirent), 1, myfs); // Writing the dirent in the data block
  fclose(myfs);
  return 0;
}
// remove a directory
int DD(char path[50]) {
  FILE* myfs = fopen("myfs", "r+"); // Opening myfs
  fseek(myfs,128 + 56,SEEK_SET);

  char *p = strtok(path,"/");
  char *name = NULL;
  while (p != NULL) {
    name = p;
    p = strtok(NULL,"/");
  }
  char *q = strtok(path,"/");
  int check = 0;
  while (q!=NULL) { // Checking if any of the directory in the path does not exist
    rewind(myfs);
    fseek(myfs,128 + 56,SEEK_SET);
    check = 0;
    for (int i = 1; i < 16; i++) {
      inode node;
		  fread(&node, sizeof(inode), 1, myfs);
      if(strcmp(node.name,q) == 0) {
        check = 1;
      }
    }
    if (check == 0 && strcmp(q,name) != 0) {
      printf("the directory %s in the given path does not exist\n",q);
      return 1;
    }
    q = strtok(NULL,"/");
  }
  int j;
  check = 0;
  rewind(myfs);
  fseek(myfs,128 + 56,SEEK_SET);
  for (int i = 1; i < 16; i++) { // Checking if the directory that we have to delete already is not there
    inode node;
		fread(&node, sizeof(inode), 1, myfs);
    if(strcmp(node.name,name) == 0) {
      check = 1;
      j = i;
    }
  }
  if (check == 0) {
    printf("%s\n","this directory does not exist");
    return 1;
  }
  // not deleting data since it will be overwritten as we have declared this node not used
  inode new;
  new.used = 0;
  strcpy(new.name, "");
  rewind(myfs);
  fseek(myfs,128 + (j*56),SEEK_SET);
  fwrite(&new, sizeof(inode), 1, myfs);
  fclose(myfs);// Closing myfs
  return 0;
}



/*
 * main
 */
int main (int argc, char* argv[]) {

  // while not EOF
    // read command
    
    // parse command
    
    // call appropriate function
    FILE * stream = fopen(argv[1], "r"); // Opening the input file
    if (stream == NULL) {
    	printf("%s%s\n","Error. Unable to open file ", argv[1]);
    	exit(1);
    }
    FILE *myfs;
    myfs = fopen ("myfs", "r");
    if (myfs == NULL) { // Opening and checking if myfs exists
      myfs = fopen ("myfs", "w"); // If it does not exist then creating one
      fseek(myfs, (1024 * 128) - 1 , SEEK_SET);
      fputc('\0', myfs);
      rewind(myfs);
      // Making the superblock
      int i = 1;
      fwrite(&i, sizeof(char), 1, myfs);
      rewind(myfs);
      fseek(myfs,128,SEEK_SET);
      // Making the root directory and its dirent
      inode root_inode;
      dirent root;
      root_inode.dir = 1;
      char name[] = "/";
      strcpy(root_inode.name,name);
      root_inode.used = 1;
      root_inode.rsvd = 0;
      root_inode.size = sizeof(dirent);
      root.inode = 0;
      root_inode.blockptrs[0] = 1;
      strcpy(root.name,name);
      root.namelen = strlen(root.name);
      fwrite(&root_inode,sizeof(inode),1,myfs);
      rewind(myfs);
      fseek(myfs, 1024, SEEK_SET);
      fwrite(&root,sizeof(dirent),1,myfs);

      printf("%s\n","myfs created");
      fclose(myfs);
    }
    else {
      printf("%s\n","myfs already exists");
      fclose(myfs);
    }
    char action[2];
    char path[50];
    char to_path[50];
    int size;
    // Reading the input file and calling appropriate functions of each command
    while(fscanf(stream,"%s",action)!=EOF) {
      if (strcmp(action,"CR") == 0) { //Create File
        fscanf(stream,"%s %d\n",path,&size);
        CR(path,size);
      }
      else if (strcmp(action,"DL") == 0) { // Delete File
        fscanf(stream,"%s\n",path);
        DL(path);
      }
      else if (strcmp(action,"CP") == 0) { // Copy File
        fscanf(stream,"%s %s\n",path,to_path);
        CP(path,to_path);
      }
      else if (strcmp(action,"MV") == 0) { // Move File
        fscanf(stream,"%s %s\n",path,to_path);
        MV(path,to_path);
      }
      else if (strcmp(action,"CD") == 0) { // Create Directory
        fscanf(stream,"%s\n",path);
        CD(path);
      }
      else if (strcmp(action,"DD") == 0) { // Remove Directory
        fscanf(stream,"%s\n",path);
        DD(path);
      }
      else if (strcmp(action,"LL") == 0) { // List all files and directories
        LL();
      }
    }
	return 0;
}
