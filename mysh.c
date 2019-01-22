#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 128

void run_command(char** args);

//parse_commands framework from given parser lab file

void parse_commands(char* line, char* segmentedCommands[], int* numSegCom) {
  // Walk through pieces of the string delimited by ampersands or semicolons
  char* pos = line;
  int counter = 0;
  
  while(1) {
    // Find the next occurrence of a semicolon or ampersand
    char* end_pos = strpbrk(pos, "&;");
    
    if(end_pos == NULL) {
      // Not found: this is the last command
      if (pos[0] == ' ') {
        memmove(pos, pos+1, strlen(pos));
      }
      segmentedCommands[counter] = pos;
      counter++;
      *numSegCom = counter;
      return;
    } else if(*end_pos == ';') {
      // Found a semicolon
      *end_pos = '\0';
      if (pos[0] == ' ') {
        memmove(pos, pos+1, strlen(pos));
      }
      segmentedCommands[counter] = pos;
      counter++;
    } else if(*end_pos == '&') {
      // Found an ampersand
      *end_pos = '\0';
      if (pos[0] == ' ') {
        memmove(pos, pos+1, strlen(pos));
      }
      //Make command start with an & to flag it for later
      char* temp = (char*)malloc (sizeof (char)* strlen(pos)+1);
      temp[0] = '&';
      temp =  strcat (temp, pos);
      segmentedCommands[counter] = temp;
      counter++;
    } else {
      // This should never happen, but just being thorough
      printf("Something strange happened!\n");
      return;
    }
    // Move the position pointer to the beginning of the next split
    pos = end_pos+1;
  }
}

int main(int argc, char** argv) {
  while(1) {
    char* line = NULL;    // Pointer that will hold the line we read in
    size_t line_length;   // Space for the length of the line
    
    // Print the shell prompt
    printf("> ");
    
    // Get a line of stdin, storing the string pointer in line and length in line_length
    if(getline(&line, &line_length, stdin) == -1) {
      if(errno == EINVAL) {
        perror("Unable to read command line");
        exit(2);
      } else {
        // Must have been end of file (ctrl+D)
        printf("\nShutting down...\n");
        exit(0);
      }
    }
    
    printf("Received command: %s\n", line);
    /*--------------------------------------------------------------------------------*/
    char * arguments[MAX_ARGS];
    char* segmentedCommands[MAX_ARGS];
    char * args, *arg;
    char * command, *com;
    int status;
    int counter = 0;
    int numSegCom = 0;
    pid_t pid;

    //Checks to see if input is blank
    if(strcmp(line,"\n")==0) {
      printf("Buffer is empty.\n");
    } else{

      parse_commands(line, segmentedCommands, &numSegCom);
      
      //Run each of the parsed commands
      for (int i =0; i<numSegCom;i++){

        //Set the first part of the command to command and strip the whitespace
        com = strtok(segmentedCommands[i], " ");
        if (com[strlen(com)-1] == '\n'){
          command = (char*)malloc(sizeof(char)*strlen(com)-1);
          command = strncpy (command, com, strlen(com)-1);
        }else{
          command = com;
        }

        //Set the rest of the command to an arguments array and strip the whitespace
        arg = strtok (NULL, " ");
  
        while (arg != NULL){
          //Strip new line from input
          if (arg[strlen(arg)-1] == '\n'){
            args = (char*)malloc(sizeof(char)*strlen(arg)-1);
            args = strncpy (args, arg, strlen(arg)-1);
          } else {
            args= arg;
          }
          arguments[counter] = args;
          arg = strtok (NULL, " "); 
          counter++;
        }
      
        /*BUILT IN COMMANDS (cd and exit)*/
				
        if (strcmp(command, "cd") ==0){
          if (chdir(arguments [0]) !=0){
            perror ("Failed cd");
          }
        } else if (strcmp (command, "exit") == 0){
          printf("Exiting \n");
          return 0;
        }

        //If it is not a built in command, open a fork and run the command with exec
        //Help in this section from https://brennan.io/2015/01/16/write-a-shell-in-c/
        else{
          pid = fork();
          if (pid < 0){
            perror("No child process\n");
            exit(1);
          } else if (pid == 0) {
           
            //Child process
             //Gets rid of & flag for background commands
            if (command[0] == '&'){
              command = memmove(command, command+1, strlen(command));
            }
            
            if (execvp(command, arguments) == -1){
              perror("Something went wrong in the child\n");
              exit(1);
              
            }
            exit (0);
           
          } else {
            //If the current command isn't a background command, wait for child to finish
            if (command[0] !='&'){
              wait(&status);
            }
            printf("Child process %d exited with status %d\n", getpid(), WEXITSTATUS(status));
          }
        }
      }
    }
    free(line);		
  }
  return 0;
}
