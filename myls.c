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
 				}
 				else if(argv[i][k] == 'l'){
 					option_l = true;
 				}
 				else if(argv[i][k] == 'R'){
 					option_R = true;
 				}
 				else{
 					fprintf(stderr, "Error: Unsupported option\n");
 					return 0;
 				}
 				k++;
 			}
 			count_options++;
 		}
 		i++;
 	}
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
 	return count_files;
 }
 
 
 void print_file_info(const char* path, const struct stat* info) { 	
	if (option_i) {
		printf("%lu ", info->st_ino);
		
		if (option_l) {
			char permissions[11];
			strcpy(permissions, get_permissions(info->st_mode));
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
		
		printf("\t%s", path);
		
		if(S_ISLNK(info->st_mode) && option_l){
			char link_target[256];
			ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
			if(len != -1){
				link_target[len] = '\0';
			}
			printf(" -> %s ", link_target);
		}
		
		printf("\n");
	}
	else if (option_l) {
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
		
		printf("\t%s", path);
		
		if(S_ISLNK(info->st_mode) && option_l){
			char link_target[256];
			ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
			if(len != -1){
				link_target[len] = '\0';
			}
			printf(" -> %s ", link_target);
		}
		
		printf("\n");
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
 	return strcasecmp(entry_a->d_name, entry_b->d_name);
 }

// seperate printing function for recursion
void print_entity(char *filePath){

	struct dirent **file_entities;
	int num_sub_dir = scandir(filePath, &file_entities, 0, compare_filenames);
	int file_desc;
	struct stat file_stat;
	struct stat info;
	 
	//if directory fails to open check if file 
	//if file exists print otherwise error
	if(num_sub_dir < 0){ 		
 		file_desc = open(filePath, O_RDONLY);
 		if(fstat(file_desc, &file_stat) < 0){
 			fprintf(stderr, "Error: Nonexistent files or directories\n");
 			return;
 		}
 		print_file_info(filePath, &file_stat);
 			 		
 		return;
 	}
	
	//if option -R valid print directory path 	
	if(option_R || filePath[0] == '/'){
		printf("%s:\n", filePath);
	}
	
	//print all files and subdirectories for current directory and recursively go through subdirectories
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
		 			continue;
		 		}
		 		
		 		lstat(currentFile, &info);
		 		
		 		if(S_ISLNK(info.st_mode)){	 		
		 			print_file_info(file_entities[j]->d_name, &info);
				}
				else{
					stat(currentFile, &info);
					print_file_info(file_entities[j]->d_name, &info);
				}
			}
		}
				
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

				char currentFile[256];
								
		 		snprintf(currentFile, sizeof(currentFile), "%s/%s", filePath, file_entities[j]->d_name);
		 		if(stat(currentFile, &info) == -1){
		 			perror("stat");
		 			free(file_entities[j]);
		 			continue;
		 		}
		 		
		 		lstat(currentFile, &info);
		 		
		 		if(S_ISLNK(info.st_mode)){	 		
		 			print_file_info(file_entities[j]->d_name, &info);
				}
				else{
					stat(currentFile, &info);
					print_file_info(file_entities[j]->d_name, &info);
				}
				
			}

			free(file_entities[j]);
		}
		
	}
	
	free(file_entities);

	return;
}
 


// our main implementation of ls command
 void ls_command(char **files, int num_files){
	for(int i = 0; i < num_files; i++){
		char *currentFile = files[i];		
		print_entity(currentFile);

	}
 }
 


 int main(int acm, char **argv){
	// NOTE: if no argument pass for directory, print the current directory
	if(acm == 1){
 		print_entity(".");
 		return 0;
 	}
 	 	
 	int num_options = counting_options(argv);
 	
	// could be changed to track actual file amount
 	char** files = (char**)(malloc(sizeof(char*) * 16));
 	for(int i = 0; i < 16; i++){
 		files[i] = (char*)(malloc(sizeof(char) * 128));
 	}
 	
 	int num_files = count_and_store_files(argv, files, num_options);
 	 	
	// if no file path pass, print current directory
	if(num_files == 0){
		strcpy(files[0], ".");
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
