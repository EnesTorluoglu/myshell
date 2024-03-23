#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
char* bookmarks[512];
int bookmarkIndex;
char* findCmd(char* path, char* cmd);
int executeCmd(char** ar, int pid_t);

int bookmark(char** args, int backgorund) {
    if(args[1] != NULL) {

        if(strcmp(args[1], "-l") == 0) {
            for(int i = 0; i < bookmarkIndex; i++) {
                printf("%d \"%s\"\n", i, bookmarks[i]);
            }
            return 1;
        }
        else if(strcmp(args[1], "-i") == 0) {
            if(args[2] == NULL) {
                printf("Missing Argument!");
                return -1;
            }
            else {//check the pid here!!!!!
                int i = args[2][0];//which bookmark is stored here
                i = i-48;//int conversion
                char* s[16];//16 cmd snippets should be enough


                //tokenize bookmarks[i] and put into s
                char bmark[MAX_LINE];//not to disrupt bookmark itself
                strcpy(bmark, bookmarks[i]);
                char* token = strtok(bmark, " ");
                int j = 0;
                while (token != NULL) {
                    s[j] = malloc(sizeof(char*)*32);
                    strcpy(s[j], token);
                    token = strtok(NULL, " ");
                    //printf("s%d: %s", j, s[j]);
                    j++;
                }
                s[j] = NULL;//HAS TO BE NULL TERMINATED FOR EXECV!!!

                long pid_t;//in general same with what we have in the main
                pid_t = fork();
                if(pid_t == -1) {
                    printf("Fork Failed!");
                }
                else if(pid_t == 0) {//execute in case of child
                    if(executeCmd(s, pid_t) == -1) {
                        printf("\nCommand execution failed!\n");
                    }
                }
                else {
                    //check backgorund here!!!
                    if(!backgorund) {
                        wait(NULL);
                    }

                }
            }
            return 1;
        }


        else if(strcmp(args[1], "-d") == 0) {
            if(args[2] == NULL) {
                printf("Missing Argument!");
                return -1;
            }
            else {//check the pid here!!!!!
                int i = args[2][0];
                i = i-48;
                if(i <= bookmarkIndex) {
                    if(i == bookmarkIndex) {
                        free(bookmarks[i]);
                        bookmarks[i] = NULL;
                    }
                    else {
                        while(bookmarks[i+1] != NULL) {
                            strcpy(bookmarks[i], bookmarks[i+1]);
                            i++;
                        }
                        free(bookmarks[bookmarkIndex]);
                        bookmarks[bookmarkIndex] = NULL;
                        bookmarkIndex--;
                    }
                }
                else {
                    printf("\nIndex too big!\n");
                    return -1;
                }
                return 1;
            }
        }
        else {//bookmarking in general is handled in this else clause
            int i = 2;//index to iterate through tokenized parts of command
            char* s = malloc(sizeof(char)*64);//string to hold the tokens
            strcat(s, args[1]);
            while(args[i] != NULL) {
                strcat(s, " ");
                strcat(s,args[i]);
                i++;
            }
            bookmarks[bookmarkIndex] = malloc(sizeof(char*)*16);
            strcpy(bookmarks[bookmarkIndex], s);
            //printf("%s",bookmarks[bookmarkIndex]);
            bookmarkIndex++;
            return 1;
        }
        return 1;
    }
    else {
        //handle error here
        printf("Missing Argument!");
        return -1;
    }
}

int removeAmpersand(char** ar) {//returns 1 for ampersand and also removes it from argument list
    int index = 0;
    while(ar[index] != NULL) {
        index++;
    }
    if(strcmp(ar[index-1], "&") == 0) {
        //printf("\ncheck here what the last is: %s\n", ar[index -1]);
        ar[index-1] = NULL;
        return 1;
    }
    else {
        return 0;
    }
}

int executeCmd(char** ar, int pid_t) {
    removeAmpersand(ar);//removes ampersand for usage
    char* path = getenv("PATH");//gets path environment variable to be used
    char* cmdPath = findCmd(path, ar[0]);//finds the relevant path for the first token in ar
    //printf("\npath: %s\ncommands: %s\n", cmdPath, ar[0]);//debugging remove later!!
    if (execv(cmdPath, ar) == 0) {//check successfull execution here
        //add error code here!
        printf("Could not execute!!");
        return -1;
    }
    return 1;
}

int cmdHandler(char** args, int backgorund) {
    if(strcasecmp(args[0], "exit") == 0) {
        //check background processes
        //notify user
        exit(0);
    }
    else if(strcasecmp(args[0], "search") == 0) {
        //impement search here!
        printf("Functionality not yet implemented!\n");
        return 1;
    }
    else if(strcasecmp(args[0], "^z") == 0) {
        //Stop running foreground action here!
        printf("Functionality not yet implemented!\n");
        return 1;
    }
    else if(strcasecmp(args[0], "bookmark") == 0) {
        if(bookmark(args, backgorund) == -1) {
            printf("Bookmark failed!");
            return -1;
        }
        return 1;
    }
    else if(strcasecmp(args[0], "cd") == 0) {
        if(args[1] == NULL) {
            printf("Missing Argument!");
            return -1;
        }
        else {
            chdir(args[1]);
            return 1;
        }
    }
    return -1;
}

char* findCmd(char* path, char* cmd) {
    DIR* dir;
    struct dirent* cd;

    char* token = strtok(path, ":");
    char* cmdPath;
    while (token != NULL) {
        dir = opendir(token);

        while((cd = readdir(dir)) != NULL) {

            if (strcmp(cd->d_name, cmd) == 0) {
                strcat(token, "/");
                cmdPath = strcat(token, cmd);
                closedir(dir);
                return cmdPath;
            }

        }
        token = strtok(NULL, ":");
    }
    return NULL;
}



/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
    i,      /* loop index for accessing inputBuffer array */
    start,  /* index where beginning of next command parameter is */
    ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    char bfr[MAX_LINE];//idk what happens here but the given printf had a problem, this fixes it in a weird way
    bfr[0] = '\0';
    strcpy(bfr, inputBuffer);
    char* tkn = strtok(bfr, "\n");
    char* str = NULL;
    str = tkn;
    strcat(str, "<<");

    printf(">>%s\n",str);//originally had inputBuffer but needed mild fixing
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
            case ' ':
            case '\t' :               /* argument separators */
                if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
                if (start != -1){
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;

            default :             /* some other character */
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&'){
                    *background  = 1;
                    inputBuffer[i-1] = '\0';
                }
        } /* end of switch */
    }    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

    for (i = 0; i <= ct; i++)
        printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

void redirect(char *fileName, int redirectionType){
    if(redirectionType == -1) return;

    // Redirect input,output or error if needed
    if (fileName != NULL) {
        if(redirectionType == 0){ // Redirect input
            int input_fd = open(fileName, O_RDONLY);
            if (input_fd == -1) {
                printf("Error: Can't open input file\n");
                exit(-1);
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        else if(redirectionType == 1){ // Redirect output
            int output_fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC);
            if (output_fd == -1) {
                printf("Error: Can't open output file\n");
                exit(-1);
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        else if(redirectionType == 2){ // Redirect append output
            int output_fd = open(fileName, O_WRONLY | O_CREAT | O_APPEND);
            if (output_fd == -1) {
                printf("Error: Can't open output file\n");
                exit(-1);
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        else if(redirectionType == 3){ // Redirect error
            int error_fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC);
            if (error_fd == -1) {
                printf("Error: Can't open error file\n");
                exit(-1);
            }
            dup2(error_fd, STDERR_FILENO);
            close(error_fd);
        }
    }
    else{
        printf("Error: No fileName found\n");
        exit(-1);
    }
}

void check_io_redirection(char **args, int argc, char *fileName) {
    // "<" input redirection = 0
    // ">" output redirection = 1
    // ">>" append redirection = 2
    // "2>" error redirection = 3
    int redirectionType = -1;

    for (int j = 0; j < argc; j++) {
        if (strcmp(args[j], "<") == 0) {
            redirectionType = 0;
            args[j] = NULL; // Remove the "<"
            if (args[j + 1] != NULL) {
                strcpy(fileName, args[j + 1]);
                j++;
            } else {
                printf("Error: Missing filename after '<'\n");
                exit(-1);
            }
        } else if (strcmp(args[j], ">") == 0) {
            redirectionType = 1;
            args[j] = NULL; // Remove the ">"
            if (args[j + 1] != NULL) {
                strcpy(fileName, args[j + 1]);
                j++;
            } else {
                printf("Error: Missing filename after '>'\n");
                exit(-1);
            }
        } else if (strcmp(args[j], ">>") == 0) {
            redirectionType = 2;
            args[j] = NULL; // Remove the ">>"
            if (args[j + 1] != NULL) {
                strcpy(fileName, args[j + 1]);
                j++;
            } else {
                printf("Error: Missing filename after '>>'\n");
                exit(-1);
            }
        } else if (strcmp(args[j], "2>") == 0) {
            redirectionType = 3;
            args[j] = NULL; // Remove the "2>"
            if (args[j + 1] != NULL) {
                strcpy(fileName, args[j + 1]);
                j++;
            } else {
                printf("Error: Missing filename after '2>'\n");
                exit(-1);
            }
        }
    }

    redirect(fileName, redirectionType);
}

int main(void)
{


    bookmarkIndex = 0;
    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */
    int index;
    while (1){
        printf("\nmyshell: ");
        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);
        cmdHandler(args, background);


        if(strcmp(args[0], "cd")) {
            long pid_t;//(1) fork a child process using fork() done here
            pid_t = fork();
            if(pid_t == -1) {
                printf("Fork Failed!");
                continue;
            }
            else if(pid_t == 0) {//execute in case of child
                if(executeCmd(args, pid_t) == -1) {//(2) the child process will invoke execv() done here
                    printf("\nCommand execution failed!\n");
                    continue;
                }
            }
            else {
                //check backgorund here!!!
                if(!background) {
                    wait(NULL);
                }
            }
        }


        /** the steps left are:
        1--io redirection
        2--^z implementation
        3--search func implementation
        */

    }
}
