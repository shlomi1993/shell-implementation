// Shlomi Ben-Shushan

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/wait.h>

#define BUFFER 256
#define SIZE 150

// JobList Data-Structure - for jobs and history built-in commands.
char jobList[SIZE][SIZE];   // 150 strings.
pid_t pids[SIZE];           // 150 PIDs
char inBackground[SIZE];    // 150 background inidicators.
int head = 0;               // Next empty index.

// For change-directory (cd) built-in command.
char OLDPWD[150];

/**********************************************************************************
* Function:     getInput
* Input:        None. Read input from the user.
* Output:       A string (char *) of the user's input.
* Operation:    Allocate memory and read char after char.
*               Reallocate if needed. Returns a pointer to the string.
***********************************************************************************/
char* getInput() {

    // Variables declaration.
    int size = BUFFER;  // Initial size is buffer's size.
    int i = 0;          // Position index.

    // Memory allocation for input string.
    char* buffer = (char *)malloc(size * sizeof(char));
    if (buffer == NULL) {
        printf("An error occurred");
        fflush(stdout);
        exit(1);
    }

    // Store input string.
    int ch;
    while (1) {

        // Read one character.
        ch = getchar();

        // If ch is EOF, replace it with a '\0' and return.
        if (ch == EOF || ch == '\n') {
            buffer[i] = '\0';
            return buffer;
        } else {
            buffer[i] = ch;
        }

        // Advance position index.
        i++;

        // If input string is longer than the buffer, reallocate memory.
        if (i >= size) {
            size += BUFFER;
            buffer = realloc(buffer, size);
            if (buffer == NULL) {
                printf("An error occurred");
                fflush(stdout);
                exit(1);
            };
        }

    }

}

/**********************************************************************************
* Function:     parse
* Input:        User's input string (char *).
* Output:       Splitted input to array of strings (char **).
* Operation:    First, this function checks for an "&" in the end to determine if
                user's command needs to be run in the background or the foreground.
                Afterwarfs, it allocates memory fror the char**, split user's
                input string to words seperated by whitespace. Reallocate if needed
                and return a pointer to the array of substrings.
***********************************************************************************/
char** parse(char* input) {

    // Check for "&" to determine running type.
    int pos = strlen(input) - 1;
    if (input[pos] == '&' && input[pos - 1] == ' ') {
        input[pos] = '\0';
        input[pos - 1] = '\0';
        inBackground[head] = 1;
    } else {
        inBackground[head] = 0;
    }

    // Save the input withput the "&" in the jobList - a history of commands that the user entered.
    strcpy(jobList[head], input);

    // Remove quotation marks if the first word is "echo".
    char inputCopy[150];
    strcpy(inputCopy, input);
    char *firstWord  = strtok(inputCopy, " "); 
    if (!strcmp(firstWord, "echo")) {
        int j = 5; // +5 for "echo ".
        while (input[j] != '\0') {
            if (input[j] == '"' || input[j] == '\'') {
                memmove(&input[j], &input[j + 1], strlen(input) - j);
            }
            j++;
        }
    }

    // Set delimiter and size of a token.
    const char *delim = " \t\r\n\a";
    int size = BUFFER;
    
    // Allocate memory for token's array.
    char **tokens = (char **)malloc(size * sizeof(char));
    if (tokens == NULL) {
        printf("An error occurred");
        fflush(stdout);
        exit(1);
    }

    // Use strtok() to get the first token.
    char *token = strtok(input, delim);

    // Splitter loop.
    int i = 0;
    while (token != NULL) {
        tokens[i] = token;
        i++;
        if (i >= size) {
            size += 100;
            tokens = realloc(tokens, size * sizeof(char *));
            if (tokens == NULL) {
                printf("An error occurred");
                fflush(stdout);
                exit(1);
            }
        }
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;

    // Return an array of tokens.
    return tokens;

}

/**********************************************************************************
* Function:     isAlive
* Input:        index i.
* Output:       1 = process i is still running. 0 = otherwise.
* Operation:    Using waitpid() function to determine if process i is stil running.
*               The i is the index of process in the Data-Structure above. the 
*               process is the i'th process in pids array.
***********************************************************************************/
int isAlive(int i) {
    if (pids[i] == 0 || waitpid(pids[i], NULL, WNOHANG))
        return 0;
    return 1;
}

/**********************************************************************************
* Function:     jobs
* Input:        None.
* Output:       1 = means continue with the main-loop.
* Operation:    This function prints every user's input (stored in jobList) That
*               is running (according to isAlive() function) and in backgroung
*               (have 1 in i'th position in inBackgrounf array). in the end, it 
*               returns 1 to continue with the main-loop.
***********************************************************************************/
int jobs() {
    int i = 0;
    while (i < head) {
        if (isAlive(i) && inBackground[i]) {
            printf("%s\n",jobList[i]);
            fflush(stdout);
        }
        i++;
    }
    return 1;
}

/**********************************************************************************
* Function:     history
* Input:        None.
* Output:       1 = means continue with the main-loop.
* Operation:    This function prints every user's input (stored in jobList) and
*               "RUNNING" if the the correspondent process is alive (according to
*               isAlive() function) or "DONE" if it isn't. in the end, it returns 1
*               to continue with the main-loop.
***********************************************************************************/
int history() {
    int i = 0;
    while (i < head) {
        if (isAlive(i))
            printf("%s RUNNING\n", jobList[i]);
        else
            printf("%s DONE\n", jobList[i]);
        fflush(stdout);
        i++;
    }
    printf("%s RUNNING\n", jobList[i]);
    fflush(stdout);
    return 1;
}

/**********************************************************************************
* Function:     cd
* Input:        char **args - user's arguments.
* Output:       1 = continue with the main-loop.
* Operation:    This function changes the current-working-directory (cwd) according
*               to the given arguments (args). It supports "~", "-" and ".." flags.
***********************************************************************************/
int cd(char **args) {

    // Save cwd at a temp variable.
    char temp[sizeof(OLDPWD)];
    getcwd(temp, sizeof(OLDPWD));   // If operation success, update OLDPWD to temp.

    // cd() can't have more than 1 additional argument.
    if (args[2] != NULL) {
        printf("Too many arguments\n");
        fflush(stdout);
    }

    // If 2nd argument is "~" or "~/", change cwd to "home" and update OLDPWD.
    else if (args[1] == NULL || !strcmp(args[1], "~") || !strcmp(args[1], "~/")) {
        if (chdir(getenv("HOME")) != 0) {
            printf("chdir failed\n");
            fflush(stdout);
        } else {
            strcpy(OLDPWD, temp);
        }
    }

    // If 2nd argument is "~/<path>"", change cwd to "home/<user>/<path>" and update OLDPWD.
    else if (args[1][0] == '~' && args[1][1] == '/') {
        char path[150];
        strcat(path, getenv("HOME"));
        strcat(path, args[1] + 1);
        if (chdir(path) != 0) {
            printf("chdir failed\n");
            fflush(stdout);
        } else {
            strcpy(OLDPWD, temp);
        }
    }

    // If 2nd argument is "-", change cwd to the previous wd and update OLDPWD.
    // Note that if OLDPWD is NULL, it will print error.
    else if (!strcmp(args[1], "-") ) {
        if (OLDPWD == NULL) {
            printf("chdir failed\n");
            fflush(stdout);
        } else {
            if (chdir(OLDPWD) != 0) {
                printf("chdir failed\n");  
                fflush(stdout);
            } else {
                strcpy(OLDPWD, temp);
            }
        }
    }

    // If 2nd argument is ".." or anything else, use change chdir original
    // fuctionality to update cwd. Than update OLDPWD.
    else {
        if (chdir(args[1]) != 0) {
            printf("chdir failed\n");
            fflush(stdout);
        } else {
            strcpy(OLDPWD, temp);
        }
    }
    
    // array to 0. Return 1 to continue with the main-loop.
    return 1;

}

/**********************************************************************************
* Function:     run
* Input:        char **args - user's input.
* Output:       1 = continue with the main-loop.
* Operation:    This function handles every command that is no built-in. It uses
*               fork and execvp as instructed in the exercise's instructions.
***********************************************************************************/
int run(char **args) {

    // Fork and save current pid in pids array.
    pid_t pid = fork();
    pids[head] = pid;
    
    // Child process.
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            printf("exec failed\n");
            fflush(stdout);
        }      
        exit(1);
    }

    // Parent process.
    else if (pid > 0) {
        if (!inBackground[head]) {
            waitpid(pid, NULL, WUNTRACED);
        }
    }

    // Case pid < 0 means fork() failed.
    else if (pid < 0) {
        printf("fork failed\n");
        fflush(stdout);
    }

    // Returns 1 to continue with the main-loop.
    return 1;

}

/**********************************************************************************
* Function:     execute
* Input:        char **args - sptted user's input without "&".
* Output:       1 = continue with the main-loop.
* Operation:    This function looks for a built-in function to run. If cannot find,
*               call run() function that handles the non-built-in functions.
***********************************************************************************/
int execute(char **args) {

    // Case empty command was entered.
    if (args == NULL || args[0] == NULL)
        return 1;

    // If the command is a builtin command, just execute it.
    // Otherwise, call run() to use fork and exec family.
    if (!strcmp(args[0], "jobs"))
        return jobs();
    if (!strcmp(args[0], "history"))
        return history();
    if (!strcmp(args[0], "cd"))
        return cd(args);
    if (!strcmp(args[0], "exit"))
        return 0;
    return run(args);

}

/**********************************************************************************
* Function:     main
* Input:        None.
* Output:       0 = to mark that this program finished correctly.
* Operation:    Entry point of the program.
***********************************************************************************/
int main() {
    int cont = 1;                   // Loop operation mark.
    char *input = NULL;             // For user's input string.
    char **args = NULL;             // For user's splitted input without "&".
    while (cont) {
        printf("$ ");
        fflush(stdout);
        input = getInput();         // Read.
        args = parse(input);        // Parse.
        cont = execute(args);       // Execute.
        free(input);
        free(args);
        head++;                     // Advance JobList Data-Structure index.
    }
    return 0;
}