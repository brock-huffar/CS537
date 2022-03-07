#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>

int interactive();
int redirect(char *str, int index);


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
  
int processString(char* str, char** parsed)
{
    char* strPipe[2];
    int redirect = 0;
  
    redirect = parsePipe(str, strPipe);
  
    if (redirect) {
        parse(strPipe[0], parsed);
        parse(strPipe[1], parsed);
        return 2;
    } else {
        parse(str, parsed);
        return 1;
    }
}

void execArgs(char** arg)
{
    pid_t pid = fork(); 
  
    if (pid == -1) {
        printf("Failed forking child\n");
        return;
    } else if (pid == 0) {
        execv("/bin/ls", arg);
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
        //skipped checking for redirect, not 100% sure that feature works but it should be close
        parse(str,parsedArgs);
        execArgs(parsedArgs);
        if (strchr(str, '>') != NULL) {
            int index = 0;
            redirect(str, index);
        }
        
    }  

  return 0; 
}

int redirect(char* str, int index) {
    char line[strlen(str)][20];
//    printf("%ld\n",strlen(str));
   // line[0] = " ";
  //  printf("%s\n", str);
    char *string;
    int i = 0;
    string = strtok (str, " >");
    while (string != NULL) {
        strcpy(line[i],string); 
        // line[i] = string;
        printf("%s\n",string);
        string = strtok(NULL, " >");
        i = i + 1;
        printf("%s\n",line[i]);
    }
    
    return 0;
}
