#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_DEFAULT_SIZE 16


/* check if the command is quit
 * strstr() ------------> check if the input contains "bye"
 * exit() --------------> exit the program
 * done
 */
void check_quit_command(char *input) {
    if (strstr(input, "bye")) {
        free(input);
        exit(0);
    }
}


/* prints the prompt
 * getenv() -----------> get username
 * getcwd() -----------> get current working directory
 * done
 */
void print_prompt() {
    char cwd[PATH_MAX];
    char *user = getenv("USER");
    if (getcwd(cwd, sizeof(cwd))) {
        printf("\033[95mcsd4980-hy345sh@%s:%s>", user, cwd);
    } else {
        printf("error getting current directory");
    }
}

/* clean the input string
 * strlen() ------------> get the length of the input
 * realloc() -----------> reallocate the buffer to the correct size
 * done
 */
void clean_string(char *input) {
	int i, start = 0, end = strlen(input) - 1;

	while (input[start] == ' ' || input[start] == '\t') {                       // remove whitespaces from the beginning
        start++;
    }
	while (end >= start && (input[end] == ' ' || input[end] == '\t')){          // remove whitespaces from the end
        end--;
    }

	for (i = start; i <= end; ++i) {                                            // shift the characters to the beginning
		input[i - start] = input[i];
	}
	input[i - start] = '\0';                                                    // add the null character at the end
}

/* reads the input dynamically
 * getchar() ------------> read the input character by character
 * malloc() -------------> allocate memory for the buffer
 * realloc() ------------> reallocate the buffer if it is full
 */
char *read_input() {
    int buffer_size = BUFFER_DEFAULT_SIZE;
    char *buffer = malloc(buffer_size * sizeof(char));
    int pos = 0;
    if (!buffer) {
        printf("malloc failed\n");
        exit(-1);
    }

    int c = getchar();

    while (c != '\n' && c != EOF) {
        buffer[pos++] = c;
        if (buffer_size == pos) {                                   // if the buffer is full
            buffer_size *= 2;                                       // double the buffer size
            buffer = realloc(buffer, buffer_size * sizeof(char));   // reallocate the buffer
            if (!buffer) {
                printf("realloc failed\n");
                exit(-1);
            }
        }
        c = getchar();                                              // read the next character
    }
    buffer[pos++] = '\0';                                             // add the null character at the end
    return realloc(buffer, pos * sizeof(char));                      // reallocate the buffer to the correct size (trims the input)
}

char **split_input(char *input, const char *needle) {
    char **elements = malloc(BUFFER_DEFAULT_SIZE * sizeof(char *));
    int buffer_size = BUFFER_DEFAULT_SIZE;
    if (!elements) {
        printf("malloc failed\n");
        exit(-1);
    }

    char *token = strtok(input, needle);
    int pos = 0;
    while (token) {
        elements[pos++] = token;
        if (pos == buffer_size) {
            buffer_size *= 2;
            elements = realloc(elements, buffer_size * sizeof(char *));
            if (!elements) {
                printf("realloc failed\n");
                exit(-1);
            }
        }
        token = strtok(NULL, needle);
    }
    elements[pos++] = NULL;
    elements = realloc(elements, pos * sizeof(char *));
    return elements;
}

void redirection_handling(char *input, int mode) {
    clean_string(input);

    if (mode == 0) {
        int fd = open(input, O_RDONLY);                                 // Read
        if (fd == -1) {
            printf("open failed\n");
            return;
        }
        dup2(fd, STDIN_FILENO);

    } else if (mode == 1) {                                             // Write
        int fd = open(input, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            printf("open failed\n");
            return;
        }
        dup2(fd, STDOUT_FILENO);

    } else if (mode == 2) {
        int fd = open(input, O_WRONLY | O_CREAT | O_APPEND, 0644);     // Append
        if (fd == -1) {
            printf("open failed\n");
            return;
        }
        dup2(fd, STDOUT_FILENO);
    }
}


void check_redirection(char *input) {                                // a < b > c       // a > b < c        // a < b >> c           // a >> b < c
    char *left;                                                     // a < b           // a > b             // a >> b
    char *right;

    if (strstr(input, "<")) {
        left = strtok(input, "<");
        right = strtok(NULL, "<");

        if (strstr(left, ">>")) {                          // append - input   (a >> b < c)    (>> <)                                   // a
            left = strtok(NULL, ">>");                             // b
            redirection_handling(left, 2);                        // left -> append
            if (right){
                redirection_handling(right, 0);                   // right -> input
            }

        } else if (strstr(right, ">>")) {                                // input - append   (a < b >> c)    (< >>)
            left = strtok(right, ">>");                           // b
            right = strtok(NULL, ">>");                           // c
            redirection_handling(left, 0);                        // left -> input
            redirection_handling(right, 2);                       // right -> append
        
        } else if (strstr(right, ">")) {                          // input - output   (a < b > c)   (<>) 
            left = strtok(right, ">");                            // b  
            right = strtok(NULL, ">");                            // c
            redirection_handling(left, 0);                        // left -> input
            redirection_handling(right, 1);                       // right -> output

        } else if (strstr(left, ">")){                           // output - input   (a > b < c)    (><)
            strtok(left, ">");                                   // a
            left = strtok(NULL, "<");                            // b         
            redirection_handling(left, 1);                       // left -> output
            redirection_handling(right, 0);                      // right -> input

        } else {                                                 // input           (a < b)         (<)
            redirection_handling(right, 0);                      // right -> input
        }

    } else if (strstr(input, ">>")) {                            // append          (a >> b)        (>>)
        strtok(input, ">>");
        right = strtok(NULL, ">>");
        redirection_handling(right, 2);                          // right -> append
    
    } else if (strstr(input, ">")) {                             // output          (a > b)         (>)
        strtok(input, ">");
        right = strtok(NULL, ">");
        redirection_handling(right, 1);                          // right -> output
    }
}

void pipe_executor(char *input) {
    char **commands = split_input(input, "|");
    int fd[2];
    int prev_output;

    for (int i = 0; commands[i]; i++) {
        check_quit_command(commands[i]);

        if (pipe(fd) == -1) {
            perror("pipe failed");
            return;
        }

        pid_t pid = fork();

        if (pid == 0) {
            if (commands[i + 1]) {
                dup2(fd[1], STDOUT_FILENO);
            }

            if (i) {
                dup2(prev_output, STDIN_FILENO);
                close(prev_output);
            }
            close(fd[0]);
            close(fd[1]);

            check_redirection(commands[i]);

            char **args = split_input(commands[i], " ");
            if (execvp(args[0], args) == -1) {
                perror("execute failed");
            }
            exit(-1);

        } else if (pid > 0) {
            close(fd[1]);
            wait(NULL);
            if (i) {
                close(prev_output);
            }
            prev_output = fd[0];
        } else {
            perror("fork failed");
        }
    }
    free(commands);
}


void exec_mul_commands(char *input) {

    check_quit_command(input);

    pid_t pid = fork();

    if (pid == 0) {

        check_redirection(input);                             // check for redirection

        char **args = split_input(input, " ");                // split the command into arguments
        
        /* fail prospatheia global variables
        while (token) {
                    if (token[0] == '$') {
                        args[args_pos++] = get_var(token + 1);
                    } else {
                        args[args_pos++] = token;
                    }
                    token = strtok(NULL, " ");
        }*/

        if (execvp(args[0], args) == -1) {                    // execute the command
            printf("execute failed\n");
        }
        exit(-1);

    } else if (pid > 0) {
        wait(NULL);
    } else {
        printf("fork failed\n");
    }
}



/* execute multiple commands
 * strstr() ------------> split the input into commands
 * done
 */
void command_checker(char *input) {
    char **commands = split_input(input, ";");

    for (int i = 0; commands[i]; i++) {
        if (strstr(commands[i], "|")) {
            pipe_executor(commands[i]);
        } else {
            exec_mul_commands(commands[i]);
        }
    }
    free(commands);
}

int main() {
    while (1) {
        print_prompt();
        char *input = read_input();
        command_checker(input);
        free(input);
    }
    return 0;
}