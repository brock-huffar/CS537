#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>
#include <stdbool.h>
#include <ctype.h>

void removeTrailingWS(char* str);
int interactive();
int redirect(char** arg, char* fileName);
int batch(char* fileName);
void trimLeading(char * str);


#define SIZE 50
typedef struct Node Node;


struct Node {
    char *alias;
    char *func;
    struct Node* next;
}*head;

struct Node* head = NULL;

int alias(char* name, char* cmd) {
    removeTrailingWS(cmd);
    // struct Node* item = (struct Node*)malloc(sizeof(struct Node));
    // item->alias = name;
    // item->func = cmd;
    // if(head == NULL){
    //     head = (struct Node*)malloc(sizeof(struct Node));
    //     *head = *item; 
    // } else {
    //     item->next = head;
    //     *head = *item;
    // }
   // printf("%s\n",head->alias);
   if(head == NULL) {
        head = (struct Node*)malloc(sizeof(struct Node));
        head->alias = malloc(strlen(name) + 1);
        strcpy(head->alias, name);
        head->func = malloc(strlen(cmd) + 1);
        strcpy(head->func, cmd);
        head->next = NULL;
   } else {
        Node* tmp = head;
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        Node *ptr = malloc(sizeof(Node));
        ptr->alias = malloc(strlen(name) + 1);
        strcpy(ptr->alias, name);
        ptr->func = malloc(strlen(cmd) + 1);
        strcpy(ptr->func, cmd);
        ptr->next = NULL;
        tmp->next = ptr;
        ptr->next = NULL;
   }
}

int freeAll(struct Node* n) {
        while (n != NULL) {
            struct Node * tmp = n;
            n=n->next;
            free(tmp);
        }
        return 0;
}

int containsAlias(struct Node* n, char* name) {
        while (n != NULL) {
            char * dummy = malloc(100);
            strcpy(dummy,n->alias);
            removeTrailingWS(dummy);
            removeTrailingWS(name);
            if (strcmp(name, n->alias) == 0) {
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
        printf("alias: Too dangerous to alias that.\n");
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

            

int findFunction(struct Node* n, char* name, char* cmd) {
        while (n != NULL) {
            char* dummy = malloc(50);
            strcpy(dummy, n->alias);
            if (strcmp(name,n->alias) == 0) {
                char *l = malloc(sizeof(n->func)+1);
                strcpy(cmd,n->func);
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
        if (strpiped[1] == NULL || *strpiped[1] == '\n' || strlen(strpiped[0]) == 0) {
            write(2, "Redirection misformatted.\n",26);
        }        
        else {
            return 1;
        }
    } else {
        return 0;
    }
}

void removeTrailingWS(char * str)
{
    int index, i, j;
    index = strlen(str)-1;
    /* Find last index of whitespace character */
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n') {
        index--;
    }
    if(index < strlen(str)-1) {
        str[index+1] = '\0'; // Make sure that string is NULL terminated
    }
//     char *end;

//   // Trim leading space
//     while(isspace((unsigned char)*str)) str++;

//     if(*str == 0) return;

//   // Trim trailing space
//     end = str + strlen(str) - 1;
//     while(end > str && isspace((unsigned char)*end)) end--;

//   // Write new null terminator character
//     end[1] = '\0';
}
  
// function for parsing command words
void parse(char* str, char** parsed)
{
    int i;
    for (i = 0; i < 100; i++) {
        // char *buf = strsep(&str, " ");
        // if (buf != NULL) {
        //     parsed[i] = buf;
        // }
        // if (buf == NULL) {
        //     break;
        // }
        parsed[i] = strsep(&str, " ");
        if (parsed[i] == NULL) break;
        // if (strlen(parsed[i]) == 0) {
        //     printf("TEST23\n");
        //     i--;
        // }
        if(isalnum(parsed[i][0])==0 && parsed[i][0] != '/') {
            trimLeading(parsed[i]);
        }
        removeTrailingWS(parsed[i]);
    }
}
  
int processString(char* str, char** parsed, char** parsedFile) {
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

void execArgs(char** arg, char* fileName) {
    pid_t pid = fork(); 
    if (pid == -1) {
        printf("Failed forking child\n");
        return;
    } else if (pid == 0) {
        if(fileName == NULL) {
            int i = execv(arg[0], arg);
            if (i == -1) {
                fprintf(stderr, "%s",arg[0]);
                write(2,": Command not found.\n",21);
            }
        } else {
            int save_out = dup(STDOUT_FILENO);
            fclose(stdout);
            FILE* fp = fopen(fileName, "w+");
            if(fp == NULL) {
                printf("Cannot write to file %s.", fileName);
            } else {
                execv(arg[0], arg);
            }
            fclose(fp);
            dup2(save_out, STDOUT_FILENO);        
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
    if(str!=NULL){
        removeTrailingWS(aliasName);
        if(checkAlias(aliasName) == 0) {  
            alias(aliasName, str);
        }
    } else {
        char* toPrint = malloc(50);
        findFunction(head,aliasName,toPrint);
        printf("%s %s\n",aliasName,toPrint);
        fflush(stdout);
    }
    return 0; 
}

int runAlias(char* name) {
    if(strcmp("alias", name) == 0){
        printAlias(head);
        return 0;
    }
    if(containsAlias(head, name) == 1) {
        removeTrailingWS(name);
        char *cmd = malloc(sizeof(char)*100);
        findFunction(head, name, cmd);
        char* runArgs[100];
        parse(cmd, runArgs);
        execArgs(runArgs, NULL);
    } else {
        printf("Alias not found");
    }
    return 0;
}

void trimLeading(char * str)
{
    int index, i, j;
    index = 0;
    //printf("%s\n", str);
    /* Find last index of whitespace character */
    while((str[index] == ' ' || str[index] == '\t' || str[index] == '\n')) {
        index++;
    }
    if(index != 0) {
        i = 0;
        while(str[i + index] != '\0')
        {
            str[i] = str[i + index];
            i++;
        }
        str[i] = '\0'; // Make sure that string is NULL terminated
    }
}

int hasAlpha(char* str) {
    for(int x = 0; x < strlen(str); x++) {
        if(isalnum(str[x])) {
            return 1;
        }
    }
    return 0;
}

int runCommand(char* str) {
    char * dummy = malloc(50);
    strcpy(dummy,str);
    trimLeading(dummy);
    if(strlen(dummy)==0) return 0;
    if(str==NULL) return 0;
    trimLeading(str);
    if(strcmp("alias",str)==0){
        runAlias(str);
    } else if(containsAlias(head,str)==1) {
        runAlias(str);
    } else if (strstr(str, "unalias") != NULL) {
        char* str2 = str;
        strsep(&str2, " ");
        if(containsAlias(head, str) ==1) {
            removeAlias(str);
        } else {
        //add print for alias not tere
        }
    } else if(strstr(str,"alias") != NULL) {
        parseAlias(str);
    } else {
        char* parsedArgs[100];
        char* parsedFileName[2];
        int redirectCheck = processString(str, parsedArgs, parsedFileName);
        if(redirectCheck == 2) {
            //this means there is a redirect
            redirect(parsedArgs, parsedFileName[0]);
        }   else {
                // int i =0;
                // while(parsedArgs[i] != NULL) {
                //     printf("%s\n",parsedArgs[i]);
                //     i++;
                // }
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
    str[511] = 'x';
    while (fgets(str,512,fptr) != NULL) {
        if(str[sizeof str - 1] == '\0' && str[sizeof str - 2] != '\n') _exit(0);
        printf("%s", str);
        fflush(stdout);
        if (strstr(str,"exit") != NULL) {
            freeAll(head);
            _exit(0);
        } else if(str != NULL) {
            // int check = hasAlpha(str);
            // if(check == 1) 
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
