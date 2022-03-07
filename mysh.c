#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int interactive();
int redirect(char *str, int index);



int main(int argc, char *argv[]) {
    if (argc == 1) {

    return interactive();
    }
}


int interactive() {
   char str[20];
   while (strcmp(str, "exit") != 0) {
        write(1, "mysh> ", 6);
        scanf("%[^\n]%*c",str);
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
