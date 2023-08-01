 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <time.h>
 #include <stdbool.h>
 #include <sys/wait.h>
 #include <sys/stat.h>
 #include <dirent.h>
 #include <grp.h>
 #include <pwd.h>


 
//define valid options
bool option_i = false;
bool option_l = false;
bool option_R = false;
 

 //go through the passed arguments and count the number of options 
 int counting_options(char **argv){
 	//define variables
 	int count_options = 0;
 	int i = 1;

 	//go through all the arguments and check if it is a valid option
 	while(argv[i] != NULL){
 		if(argv[i][0] == '-'){
 			int k = 1;
 			while(argv[i][k]){
 				if(argv[i][k] == 'i'){
 					option_i = true;
 					//count_options++;
 				}
 				else if(argv[i][k] == 'l'){
 					option_l = true;
 					//count_options++;
 				}
 				else if(argv[i][k] == 'R'){
 					option_R = true;
 					//count_options++;
 				}
 				else{
 					fprintf(stderr, "Error: Unsupported option\n");
 					return 0;
 				}
 				k++;
 			}
 			count_options++;
 		}
 		// printf("argv[i] is: %s\n", argv[i]);
 		i++;
 	}
 	// printf("count_options is: %d\n", count_options);
 	return count_options;
 }
 

 //go through the passed arguments and count the number of files and store them
 int count_and_store_files(char **argv, char **files, int num_options){
 	int count_files = 0;
 	int i = num_options + 1;
 	int j = 0;
 	
 	while(argv[i] != NULL){
 		//increment count
 		count_files++;
 		
 		//store each path in files
 		strcpy(files[j], argv[i]);
 		i++;
 		j++;
 	}
 	// printf("count_files is: %d\n", count_files);
 	return count_files;
 }
 


// seperate printing function for recursion
void print_entity(char *filePath){

	struct dirent **file_entities;
	struct stat file_info;
	int num_sub_dir = scandir(filePath, &file_entities, 0, alphasort);

	// if directory fails to open
	if(num_sub_dir < 0){
		printf("Error	: Nonexistent files or directories\n");
		return;
	}

	printf("%s:\n", filePath);

	for(int j = 0; j < num_sub_dir; j++){
		if(strcmp(file_entities[j]->d_name, ".") != 0 && strcmp(file_entities[j]->d_name, "..") != 0){
			// int length = strlen(currentFile) + strlen(file_entities[j]->d_name) + 1;
			// char *name = malloc(length * sizeof(char));

			char currentFile[256];
			strcpy(currentFile, filePath);
			strcat(currentFile, "/");
			strcat(currentFile, file_entities[j]->d_name);
			stat(currentFile, &file_info);
			struct passwd *pw = getpwuid(file_info.st_uid);
			struct group *gr = getgrgid(file_info.st_gid);

			if(option_l){
				if(option_i){
					printf("%lu \t", file_info.st_ino);
				}

				// we modify this part from other's code on stackoverflow
				printf((S_ISDIR(file_info.st_mode)) ? "d" : "-");
				printf((file_info.st_mode &  S_IRUSR) ? "r" : "-");
				printf((file_info.st_mode &  S_IWUSR) ? "w" : "-");
				printf((file_info.st_mode &  S_IXUSR) ? "x" : "-");
				printf((file_info.st_mode &  S_IRGRP) ? "r" : "-");
				printf((file_info.st_mode &  S_IWGRP) ? "w" : "-");
				printf((file_info.st_mode &  S_IXGRP) ? "x" : "-");
				printf((file_info.st_mode &  S_IROTH) ? "r" : "-");
				printf((file_info.st_mode &  S_IWOTH) ? "w" : "-");
				printf((file_info.st_mode &  S_IXOTH) ? "x" : "-");
				printf("\t%ld",file_info.st_nlink);
				printf("\t%ld", file_info.st_size);
				printf("\t%s", pw->pw_name);
				printf("\t%s", gr->gr_name);
				printf("\t%s", ctime(&file_info.st_mtime));
				printf("\t%s", file_entities[j]->d_name);
				printf("\n");

			}else if(option_i){
				printf("%lu \t%s\n", file_info.st_ino, file_entities[j]->d_name);
			}else{
				printf("%s\n", file_entities[j]->d_name);
			}

			// if recursive and file is of type directory
			if(option_R && file_entities[j]->d_type == DT_DIR){
				printf("\n");
				print_entity(currentFile);
			}
		}

		free(file_entities[j]);
	}

	free(file_entities);

	return;
}
 


// our main implementation of ls command
 void ls_command(char **files, int num_files){

	for(int i = 0; i < num_files; i++){

		char *currentFile = files[i];
		
		print_entity(currentFile);

		printf("\n");
	}
 }
 


 int main(int acm, char **argv){
 	//no arguments passed, list files in current directory

	// NOTE: if no argument pass for directory, print the current directory
 	if(acm < 2){
 		printf("Error: incorrect argument amount\n");
 		return 0;
 	}
 	
 	int num_options = counting_options(argv);
 	
	// could be changed to track actual file amount
 	char** files = (char**)(malloc(sizeof(char*) * 16));
 	for(int i = 0; i < 16; i++){
 		files[i] = (char*)(malloc(sizeof(char) * 128));
 	}
 	
 	int num_files = count_and_store_files(argv, files, num_options);
 	
 	// for(int i = 0; i < num_files; i++){
 	// 	printf("files[i] is: %s\n", files[i]);
 	// }
 	
	// if no file path pass, print current directory
	if(num_files == 0){
		
		files[0] = ".";
		ls_command(files, 1);
		
	}else{
		ls_command(files, num_files);
		for(int i = 0; i < 16; i++){
			free(files[i]);
		}
	}

	free(files);

 	return 0;
 }
