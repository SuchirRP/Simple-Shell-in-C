#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//input parsing function
void lsh_loop(void);
char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_launch(char **args);
//function to execute the parsed commands
int lsh_execute(char **args);
//functions for inbuilt shellcommands
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
//list of builtin commands
char *builtin_str[] = {"cd", "help", "exit"};
int (*builtin_func[])(char**) = {&lsh_cd, &lsh_help, &lsh_exit};
int lsh_num_builtins() {
    return sizeof(builtin_str)/sizeof(char*);
}
//builtin function implementations
int lsh_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "lsh: expected arrgument \"cd\" \n");
    }
    else {
        if(chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args) {
    printf("Suchir Punuru\'s LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are the built in: \n");

    for(int i = 0; i < lsh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use man command for info on other commands.\n");
    return 1;
}

int lsh_exit(char **args) {
    return 0;
}

//main body
int main(int argc, char **argv) {
    //Load config files

    //Command Loop
    lsh_loop();

    //Perform any shutdown/cleanup
    return EXIT_SUCCESS;
}

int lsh_execute(char **args) {
    if(args[0] == NULL) {
        //NULL implies no arguments were given
    }
    
    for(int i = 0; i < lsh_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args);
}

//input parsing implemenation
void lsh_loop(void) {
    char *line;
    char**args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while(status);
}

char *lsh_read_line(void) {
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "lsh: allocation error \n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        c = getchar();

        if(c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position] = c;
        }
        position++;

        if(position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer) {
                fprintf(stderr, "lsh: allocation error \n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **lsh_split_line(char *line) {
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens) {
        fprintf(stderr, "lsh: allocation error \n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if(position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "lsh: allocation error \n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();

    if(pid == 0) {
        //child process
        if(execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0) {
        //error forking
        perror("lsh");
    }
    else {
        //Parent proccess
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}
