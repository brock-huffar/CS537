#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>
#include <stdbool.h>


int interactive();
int redirect(char** arg, char* fileName);

#define SIZE 50



struct Node {
    char* alias;
    char* func;
    struct Node* next;
};

struct Node* head = NULL;
struct Node* tail = (struct Node*)malloc(sizeof(structNode));



struct DataItem *search(int key) {
   //get the hash 
   int hashIndex = hashCode(key);  
	
   //move in array until an empty 
   while(hashArray[hashIndex] != NULL) {
	
      if(hashArray[hashIndex]->key == key)
         return hashArray[hashIndex]; 
			
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
   }        
	
   return NULL;        
}


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
            printf("%s",parsedFileName[0]);
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


int alias(char** str) {
    char *alias[50];
    alias = str[1];
    char *command[50];
    command = str[2];
    struct Node* next = NULL;
    if (head == NULL) {
        head = (struct Node*)malloc(sizeof(struct Node));
        head->alias = alias;
        head->func = command;
        head->next = tail;
        }
    else if (head->next == tail) {
        next = (struct Node*)malloc(sizeof(struct Node));
        head->next = next;
        next->alias = alias;
        next->func = command;
        next->next = tail;
    }
    else {
        next = (struct Node*)malloc(sizeof(struct Node));
        next->alias = alias;
        next->func = command;
        tail=next;
        tail->next = null;
    }
}

int checkAlias(char* alias) {
    if (strcmp(alias, "alias") == 0) {
        printf("alias: Too dangerous to alias that.\n");
        return 1;
    if (strcmp(alias, "unalias") == 0) {
        printf("alias: Too dangerous to alias that.\n");
        return 1;
    }
    if (strcmp(alias, "exit") == 0) {
        printf("alias: Tpp dangerous to alias that.\n");
        return 1;
        }
    

    return 0;

}

int remove(struct Node* n, char* alias) {
        if (containsAlias(n, alias) == 1) {
            while (n != NULL) {
                if (strcmp(n->alias,alias) == 0) {
                    n->alias = "brockhuffar";
                    return 0;
                }
                n = n->next;
            }
        }
        return 1;
}

int replace(struct Node* n, char* alias, char* func) {
        if (containsAlias(n, alias) == 1) {
            while (n != NULL) {
                if (strcmp(n->alias,alias) == 0) {
                    n->func = func;
                    return 0;            
                }
                n = n->next;
            }
        }
        return 1;
    }

            

const char* findFunction(struct Node* n, char* alias) {
        while (n != NULL) {
            if (strcmp(n->alias,alias) == 0) {
                return n->func;
            }
            n = n->next;
        }
        return NULL;
    }


int containAlias(struct Node* n, char* alias) {
        while (n != NULL) {
            if (strcmp(n->alias,alias) == 0) {
                return 1;
            }
            n = n->next;
            }
        return 0;
    }

int printAlias(struct Node* n) {
    while (n != NULL) {
        printf("%s\n",n->alias);
        n = n->next;
    }
    return 0;
}



