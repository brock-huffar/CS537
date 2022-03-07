#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>

int interactive();
int redirect(char** arg, char* fileName);


// function for finding pipe
int parsePipe(char* string, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&string, ">");
        if (strpiped[i] == NULL)
            break;
    }
  
    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}
  
// function for parsing command words
void parse(char* str, char** parsed)
{
    int i;
  
    // need to fix loop to prevent segfault
    for (i = 0; i < 100; i++) {
        parsed[i] = strsep(&str, " ");
  
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}
  
int processString(char* str, char** parsed, char** parsedFile)
{
    char* strPipe[2];
    int redirect = 0;
    redirect = parsePipe(str, strPipe);
  
    if (redirect) {
        parse(strPipe[0], parsed);
        parse(strPipe[1], parsedFile);
        return 2;
    } else {
        parse(str, parsed);
        return 1;
    }
}

void execArgs(char** arg, char* fileName)
{
    pid_t pid = fork(); 
  
    if (pid == -1) {
        printf("Failed forking child\n");
        return;
    } else if (pid == 0) {
        if(fileName == NULL) {
            execv("/bin/ls", arg);
        } else {
            fclose(stdout);
            FILE* fp = fopen(fileName, "w");
            if(fp == NULL) {
                printf("Cannot write to file %s.", fileName);
            } else {
                execv("/bin/ls", arg);
            }            
        }
        _exit(0);
    } else {
        wait(NULL); 
        return;
    }
}



int main(int argc, char *argv[]) {
    interactive();
}


int interactive() {
    char str[50];
    while (strcmp(str, "exit") != 0) {
        write(1, "mysh> ", 6);
        scanf("%[^\n]%*c",str);
        if(strcmp(str,"exit") ==0) {
            exit(0);
        }
        char* parsedArgs[100];
        char* parsedFileName[2];
        int redirectCheck = processString(str, parsedArgs, parsedFileName);
        if(redirectCheck == 2) {
            //this means there is a redirect
            redirect(parsedArgs, parsedFileName[0]);
        } else {
            // no redirect
            execArgs(parsedArgs, NULL);
        }
    }  

  return 0; 
}

int redirect(char** proc, char* file) {
    execArgs(proc, file);
    // const char* filename = get_filename_or_null();
    // if(filename) {
    // int fd = open(filename, O_WRONLY, 0666);
    // dup2(fd, STDOUT_FILENO); // Check `man stdin` for more info
    // dup2(fd, STDERR_FILENO);
    // // Check the return values of dup2 here
    // }

    // // Filler code here
    // printf("My fancy output");

    
    return 0;
}
