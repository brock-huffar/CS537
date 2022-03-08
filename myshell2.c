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
int batch(char* fileName);


#define SIZE 50



struct Node {
    char* alias;
    char* func;
    struct Node* next;
};

struct Node* head = NULL;

int alias(char* name, char* cmd) {
    struct Node* item = (struct Node*)malloc(sizeof(struct Node));
    item->alias = name;
    item->func = cmd;
    if(head != NULL){
        item->next = head;
    }
    head = item;
}

int containsAlias(struct Node* n, char* alias) {
        while (n != NULL) {
            if (strcmp(n->alias,alias) == 0) {
                return 1;
            }
            n = n->next;
            }
        return 0;
    }

int checkAlias(char* alias) {
    if (strcmp(alias, "alias") == 0) {
        printf("alias: Too dangerous to alias that.\n");
        return 1;
    }
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

int removeAlias(char* alias) {
    struct Node* curr = head;
        while (curr != NULL) {
            if (strcmp(curr->alias,alias) == 0) {
                curr->alias = "brockhuffar";
                return 0;
            }
            curr = curr->next;
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

            

char* findFunction(struct Node* n, char* alias) {
        while (n != NULL) {
            if (strcmp(n->alias,alias) == 0) {
                return n->func;
            }
            n = n->next;
        }
        return NULL;
    }




int printAlias(struct Node* n) {
    while (n != NULL) {
        printf("%s\n",n->alias);
        n = n->next;
    }
    return 0;
}

// function for finding pipe
int parsePipe(char* string, char** strpiped)
{
    if(strstr(string, ">") != NULL){
        int i;
        for (i = 0; i < 2; i++) {
            strpiped[i] = strsep(&string, ">");
            if (strpiped[i] == NULL)
                break;
        }
        int count = 0;
        for(int i = 0; i < strlen(strpiped[1]); i++){
            if(strpiped[1][i]==' ') {
                count++;
            }
        } if(count>1) {
            write(2, "Redirection misformatted.\n",26);
        }
        if (strpiped[1] == NULL || *strpiped[1] == '\n') {
            write(2, "Redirection misformatted.\n",26);
        }
        
        else {
            return 1;
        }
    } else {
        return 0;
    }
}
  
// function for parsing command words
void parse(char* str, char** parsed)
{
    int i;
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
    char* cmd;
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
            int i = execv(arg[0], arg);
            if (i == -1) {
                write(2,"job: Command not found.\n",25);
            }
        } else {
            fclose(stdout);
            FILE* fp = fopen(fileName, "w");
            if(fp == NULL) {
                printf("Cannot write to file %s.", fileName);
            } else {
                execv(arg[0], arg);
            }            
        }
        _exit(0);
    } else {
        wait(NULL); 
        return;
    }
}



int main(int argc, char *argv[]) {
    if(argc == 1) {   
        interactive();
    } else if (argc >2){
        write(2,"Usage: mysh [batch-file]\n",25);
        _exit(1);
    } else {
        batch(argv[1]);
    }
}

int parseAlias(char* str) {
    strsep(&str, " ");
    char * aliasName = strsep(&str, " ");
    if(checkAlias(aliasName) == 0) {  
        alias(aliasName, str);
    }
    return 0; 
}

int runAlias(char* name) {
    if(strcmp("alias", name) == 0){
        printAlias(head);
        return 0;
    }
    if(containsAlias(head, name) == 1) {
        char * cmd = findFunction(head, name);
        char* runArgs[100];
        parse(cmd, runArgs);
        execArgs(runArgs, NULL);
    } else {
        printf("Alias not found");
    }
    return 0;
}

int runCommand(char* str) {
    if(strstr(str, " ") == NULL && containsAlias(head, str)==1){
        runAlias(str);
    } else if(strstr(str,"alias") != NULL) {
        parseAlias(str);
    } else if (strstr(str, "unalias") != NULL) {
        char* str2 = str;
        strsep(&str2, " ");
        if(containsAlias(head, str) ==1) {
            removeAlias(str);
        } else {
        //add print for alias not tere
        }
    } else {
        char* parsedArgs[100];
        char* parsedFileName[2];
        int redirectCheck = processString(str, parsedArgs, parsedFileName);
        if(redirectCheck == 2) {
            //this means there is a redirect
            redirect(parsedArgs, parsedFileName[0]);
        }   else {
                int i =0;
                while(parsedArgs[i] != NULL) {
                    printf("%s\n",parsedArgs[i]);
                    i++;
                }
                execArgs(parsedArgs, NULL);
        }
    }

}

int interactive() {
    char str[50];
    while (strcmp(str, "exit") != 0) {
        write(1, "mysh> ", 6);
        if(scanf("%[^\n]%*c",str) <= 0) {
            _exit(0);
        }
        if(strcmp(str,"exit") ==0) {
             exit(0);
        }
        runCommand(str);
    }
  return 0; 
}

int batch(char* fileName) {
    FILE *fptr;

    if ((fptr = fopen (fileName, "r")) == NULL) {
        fprintf(stderr,"Error: Cannot open file %s.\n",fileName);
        fflush(stdout);
        _exit(1);
    }
    char str[512];
    while (fgets(str,512,fptr) != NULL) {
        printf("%s", str);
        fflush(stdout);
        if (strstr(str,"exit") != NULL) {
            _exit(0);
        } else if(str != NULL) {
            runCommand(str);
        }
    }
    _exit(0);
    return 0;
}


int redirect(char** proc, char* file) {
    execArgs(proc, file);
    return 0;
}
