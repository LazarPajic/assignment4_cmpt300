 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <time.h>
 #include <stdbool.h>
 #include <sys/wait.h>
 
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
 					option_i == true;
 					//count_options++;
 				}
 				else if(argv[i][k] == 'l'){
 					option_l == true;
 					//count_options++;
 				}
 				else if(argv[i][k] == 'R'){
 					option_R == true;
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
 		printf("argv[i] is: %s\n", argv[i]);
 		i++;
 	}
 	printf("count_options is: %d\n", count_options);
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
 	printf("count_files is: %d\n", count_files);
 	return count_files;
 }
 
 
 void ls_command(char **files, int num_files){
 	
 }
 
 int main(int acm, char **argv){
 	//no arguments passed, list files in current directory
 	if(acm == 1){
 		
 		return 0;
 	}
 	
 	int num_options = counting_options(argv);
 	
 	char** files = (char**)(malloc(sizeof(char*) * 16));
 	for(int i = 0; i < 16; i++){
 		files[i] = (char*)(malloc(sizeof(char) * 128));
 	}
 	
 	int num_files = count_and_store_files(argv, files, num_options);
 	
 	for(int i = 0; i < num_files; i++){
 		printf("files[i] is: %s\n", files[i]);
 	}
 	
 	ls_command(files, num_files);
 	
 	for(int i = 0; i < 16; i++){
 		free(files[i]);
 	}
 	free(files);
 	return 0;
 }
