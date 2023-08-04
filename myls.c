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
#include <fcntl.h>

//declare function
 const char* get_permissions(mode_t mode);
 
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
 		//printf("./* is %s\n", argv[i]);
 		//store each path in files
 		strcpy(files[j], argv[i]);
 		i++;
 		j++;
 	}
 	// printf("count_files is: %d\n", count_files);
 	return count_files;
 }
 

//testing*****************************************************************************************************
 
 void print_file_info(const char* path, const struct stat* info) { 	
	if (option_i) {
		printf("%lu ", info->st_ino);
		
		if (option_l) {
			//printf("did it enter option l");
			char permissions[11];
			strcpy(permissions, get_permissions(info->st_mode));
			//printf("%s ", get_permissions(info->st_mode));
			printf("\t%s ", permissions);
			printf("\t%lu ", info->st_nlink);

			struct passwd* pwd = getpwuid(info->st_uid);
			printf("\t%s ", pwd ? pwd->pw_name : "unknown");

			struct group* grp = getgrgid(info->st_gid);
			printf("\t%s ", grp ? grp->gr_name : "unknown");

			printf("\t%ld ", (long)info->st_size);

			struct tm* timeinfo = localtime(&info->st_mtime);
			char buffer[80];
			strftime(buffer, sizeof(buffer), "%b %d %Y %H:%M", timeinfo);			
			printf("\t%s ", buffer);
		}
		
		printf("\t%s\n", path);
		
		if(S_ISLNK(info->st_mode) && option_l){
			char link_target[256];
			ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
			if(len != -1){
				link_target[len] = '\0';
			}
			printf("-> %s ", link_target);
		}
	}
	//printf("did it enter option l %d", option_l);
	else if (option_l) {
		//printf("did it enter option l");
		char permissions[11];
		strcpy(permissions, get_permissions(info->st_mode));
		printf("%s ", permissions);
		printf("\t%lu ", info->st_nlink);

		struct passwd* pwd = getpwuid(info->st_uid);
		printf("\t%s ", pwd ? pwd->pw_name : "unknown");

		struct group* grp = getgrgid(info->st_gid);
		printf("\t%s ", grp ? grp->gr_name : "unknown");

		printf("\t%ld ", (long)info->st_size);

		struct tm* timeinfo = localtime(&info->st_mtime);
		char buffer[80];
		strftime(buffer, sizeof(buffer), "%b %d %Y %H:%M", timeinfo);
		printf("\t%s ", buffer);
		
		printf("\t%s\n", path);
	}
	else{
		printf("%s\n", path);
	}
}

const char* get_permissions(mode_t mode) {
	static char permissions[11];

	permissions[0] = (S_ISLNK(mode)) ? 'l' : (S_ISDIR(mode)) ? 'd' : '-';
	permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
	permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
	permissions[3] = (mode & S_IXUSR) ? 'x' : '-';
	permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
	permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
	permissions[6] = (mode & S_IXGRP) ? 'x' : '-';
	permissions[7] = (mode & S_IROTH) ? 'r' : '-';
	permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
	permissions[9] = (mode & S_IXOTH) ? 'x' : '-';
	permissions[10] = '\0';

	return permissions;
}

 int compare_filenames(const void* a, const void* b){
 	const struct dirent* entry_a = *(const struct dirent**)a;
 	const struct dirent* entry_b = *(const struct dirent**)b;
 	//printf("test %s\n", entry_a->d_name);
 	//printf("test 2 %s\n", entry_b->d_name);
 	return strcasecmp(entry_a->d_name, entry_b->d_name);
 }
 //testing*****************************************************************************************************

// seperate printing function for recursion
void print_entity(char *filePath){

	struct dirent **file_entities;
	//struct stat file_info;
	int num_sub_dir = scandir(filePath, &file_entities, 0, compare_filenames);
	int file_desc;
	struct stat file_stat;
	struct stat info;
	//struct stat entry_stat;
	 
	// if directory fails to open
	/*if(num_sub_dir < 0){
		printf("Error	: Nonexistent files or directories\n");
		return;
	}*/
	//if directory fails to open check if file 
	//if file exists print otherwise error
	if(num_sub_dir < 0){ 		
 		file_desc = open(filePath, O_RDONLY);
 		if(fstat(file_desc, &file_stat) < 0){
 			fprintf(stderr, "Error: Nonexistent files or directories\n");
 			return;
 		}
 		print_file_info(filePath, &file_stat);
 		//printf("%-10s", path);
 		 		
 		return;
 	}
 	//printf("test %s\n", filePath);
	//if(strcmp(filePath, ".") != 0){
	//if(option_R){
	printf("%s:\n", filePath);
	//}
	
	
	if(option_R){
		for(int j = 0; j < num_sub_dir; j++){
			if(strcmp(file_entities[j]->d_name, ".") != 0 && strcmp(file_entities[j]->d_name, "..") != 0){
				if(file_entities[j]->d_name[0] == '.'){
		 			continue;
		 		}

				char currentFile[256];
				
		 		snprintf(currentFile, sizeof(currentFile), "%s/%s", filePath, file_entities[j]->d_name);
		 		if(stat(currentFile, &info) == -1){
		 			perror("stat");
		 			//free(file_entities[j]);
		 			continue;
		 		}
		 		
		 		print_file_info(file_entities[j]->d_name, &info);
			}
		}
		
		//struct dirent **file_entities;
		for(int i = 0; i < num_sub_dir; i++){
			if(file_entities[i]->d_name[0] == '.'){
	 			free(file_entities[i]);
	 			continue;
	 		}
	 		char currentFile[256];
	 		snprintf(currentFile, sizeof(currentFile), "%s/%s", filePath, file_entities[i]->d_name);
	 		if(stat(currentFile, &info) == -1){
	 			perror("stat");
	 			free(file_entities[i]);
	 			continue;
	 		}
	 		// if recursive and file is of type directory
			if(option_R && file_entities[i]->d_type == DT_DIR && S_ISDIR(info.st_mode) && strcmp(file_entities[i]->d_name, ".") != 0 && strcmp(file_entities[i]->d_name, "..") != 0){
				printf("\n");
				print_entity(currentFile);
			}
			free(file_entities[i]);
		}
	}
	else{
		//print all files and subdirectories for current directory
		for(int j = 0; j < num_sub_dir; j++){
			if(strcmp(file_entities[j]->d_name, ".") != 0 && strcmp(file_entities[j]->d_name, "..") != 0){
				if(file_entities[j]->d_name[0] == '.'){
		 			free(file_entities[j]);
		 			continue;
		 		}
				// int length = strlen(currentFile) + strlen(file_entities[j]->d_name) + 1;
				// char *name = malloc(length * sizeof(char));

				char currentFile[256];
				/*strcpy(currentFile, filePath);
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
				}*/
				
		 		snprintf(currentFile, sizeof(currentFile), "%s/%s", filePath, file_entities[j]->d_name);
		 		if(stat(currentFile, &info) == -1){
		 			perror("stat");
		 			free(file_entities[j]);
		 			continue;
		 		}
		 				 		
		 		print_file_info(file_entities[j]->d_name, &info);

				// if recursive and file is of type directory
				/*if(option_R && file_entities[j]->d_type == DT_DIR && S_ISDIR(info.st_mode) && strcmp(file_entities[j]->d_name, ".") != 0 && strcmp(file_entities[j]->d_name, "..") != 0){
					printf("\n");
					print_entity(currentFile);
				}*/
			}

			free(file_entities[j]);
		}
		
		//free(file_entities);
	}
	
	free(file_entities);

	return;
}
 


// our main implementation of ls command
 void ls_command(char **files, int num_files){
	//printf("num files %d\n", num_files);
	for(int i = 0; i < num_files; i++){

		char *currentFile = files[i];
		
		print_entity(currentFile);

		//printf("\n");
	}
 }
 


 int main(int acm, char **argv){
 	//no arguments passed, list files in current directory

	// NOTE: if no argument pass for directory, print the current directory
	if(acm == 1){
 		print_entity(".");
 		return 0;
 	}
 	/*
 	if(acm < 2){
 		printf("Error: incorrect argument amount\n");
 		return 0;
 	}*/
 	
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
		strcpy(files[0], ".");
		//files[0] = ".";
		ls_command(files, 1);
		
	}else{
		ls_command(files, num_files);
	}
	
	for(int i = 0; i < 16; i++){
		free(files[i]);
	}	
	free(files);

 	return 0;
 }
