#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define PROMPT "lambda-shell$ "

#define MAX_TOKENS 100
#define COMMANDLINE_BUFSIZE 1024
#define DEBUG 1  // Set to 1 to turn on some debugging output, or 0 to turn off

// My little baby functions. Aww, so cute.
char **parse_commandline(char *str, char **args, int *args_count);
void doCD(char** args, int args_count);
void flagSpecialArgs(char** args, int args_count, int *background, int *redirection);

/**
 * Main
 */
int main(void)
{
    // Holds the command line the user types in
    char commandline[COMMANDLINE_BUFSIZE];

    // Holds the parsed version of the command line
    char *args[MAX_TOKENS];

    // How many command line args the user typed
    int args_count;

    // Shell loops forever (until we tell it to exit)
    while (1) {
        // Command modifier flags
        int background = 0;
        int redirection = 0;

        // Clear out the zombies
        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;

        // Print a prompt
        printf("%s", PROMPT);
        fflush(stdout); // Force the line above to print

        // Read input from keyboard
        fgets(commandline, sizeof commandline, stdin);

        // Exit the shell on End-Of-File (CRTL-D)
        if (feof(stdin)) {
            break;
        }

        // Parse input into individual arguments
        parse_commandline(commandline, args, &args_count);

        if (args_count == 0) {
            // If the user entered no commands, do nothing
            continue;
        }

        // Exit the shell if args[0] is the built-in "exit" command
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        #if DEBUG

        // Some debugging output

        // Print out the parsed command line in args[]
        for (int i = 0; args[i] != NULL; i++) {
            printf("%d: '%s'\n", i, args[i]);
        }

        #endif
        
        flagSpecialArgs(args, args_count, &background, &redirection);

        /* Add your code for implementing the shell's logic here */
        int rc = fork();

        if (rc < 0) {
            fprintf(stderr, "Fork unsuccessful. Perhaps try using a knife?\n");
            exit(1);
        }
        else if (rc == 0) {
            // If the command is `cd`, we'll take care of it instead of exec()
            if (strcmp(args[0], "cd") == 0) {
                doCD(args, args_count);
                continue;
            }
            //
            // The actual execution of the program
            execvp(args[0], args);
        }
        else {
            printf("PARENT background %d\n", background);
            if (!background) {
                printf("BOO!\n");
                waitpid(rc, NULL, 0);
            }
        }
    }

    return 0;
}

void doCD(char** args, int args_count)
{
    if (args_count != 2) {
        fprintf(stderr, "usage: cd <directory>\n");
    }
    else {
        // Execution is in the if conditional
        // Error handling accomplished by checking its return value
        if (chdir(args[1]) == -1) {
            perror("chdir");
        }
    }
}

void flagSpecialArgs(char* args[], int args_count, int *background, int *redirection)
{
    int lastCharInd = args_count - 1;

    if (args[lastCharInd][0] == '&') {
        *background = 1;
        args[lastCharInd] = NULL;
    }
    // if (args[lastCharInd][0] == '>') {
    //     *redirection = 1;
    //     args[lastCharInd] = NULL;
    // }
    
}

/**
 * Parse the command line.
 *
 * YOU DON'T NEED TO MODIFY THIS!
 * (But you should study it to see how it works)
 *
 * Takes a string like "ls -la .." and breaks it down into an array of pointers
 * to strings like this:
 *
 *   args[0] ---> "ls"
 *   args[1] ---> "-la"
 *   args[2] ---> ".."
 *   args[3] ---> NULL (NULL is a pointer to address 0)
 *
 * @param str {char *} Pointer to the complete command line string.
 * @param args {char **} Pointer to an array of strings. This will hold the result.
 * @param args_count {int *} Pointer to an int that will hold the final args count.
 *
 * @returns A copy of args for convenience.
 */
char **parse_commandline(char *str, char **args, int *args_count)
{
    char *token;
    
    *args_count = 0;

    token = strtok(str, " \t\n\r");

    while (token != NULL && *args_count < MAX_TOKENS - 1) {
        args[(*args_count)++] = token;

        token = strtok(NULL, " \t\n\r");
    }

    args[*args_count] = NULL;

    return args;
}
