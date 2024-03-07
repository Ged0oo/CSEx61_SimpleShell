#include "main.h"


// Global variable to store command history
char hist[MAX_HISTORY][MAX_LENGTH];
int history_count = 0;


int main()     
{
        printf("\n********************************************\n");
        printf("*********** This is Terminal. **************\n");
        printf("********************************************\n\n");
        register_child_signal(on_child_exit);
        setup_environment();
        runShell();
        return 0;
}


/*
 * Function to register signal handler for child exit
 * This function registers a signal handler for the SIGCHLD signal, 
 * which is sent to the parent process when a child process terminates.
 */
void register_child_signal(void (*on_child_exit)(int)) 
{
    signal(SIGCHLD, on_child_exit);
}


/*
 * Function to set up the environment
 * This function changes the current directory
 * to the present working directory.
 */
void setup_environment() 
{
    chdir(getenv("PWD"));
}


/*
 * Function to handle child exit
 * This function is called when a child process terminates
 * and it reaps any zombie child processes.
 */
void on_child_exit()
{
    reap_child_zombie();
	// Write to log file that a child process terminated
    WriteLogFile("Child terminated\n");
}


/*
 * Function to reap zombie child processes
 * This function waits for and reaps any zombie
 * child processes to avoid accumulating zombie processes.
 */
void reap_child_zombie()
{
	// returns immediately if there are no child processes 
	// that have exited and the loop continues until waitpid() returns 0,
    // indicating that there are no more child processes to reap. 
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0);
}


/*
 * Function to write a line to a log file
 * This function opens the specified log file
 * in append mode and writes the given line to it.
 */
void WriteLogFile(char line[])
{
    FILE *log;

    log = fopen(LOGS, "a");
    if (log == NULL)
        return;

    fprintf(log, "%s", line);
    fclose(log);
}


/*
 * Function to run the shell
 * This function continuously prompts the user for input, 
 * parses and evaluates the input, and executes the corresponding commands until the user exits.
 */
void runShell()                                                                  
{
        int flag = 1;
        do
        {
                input_t input = builtin;
                char cmd[MAX_LENGTH] = {};
                char *argv[MAX_ARGS] = {};

                ReadInput(cmd);
                if(cmd[0] == '\0')
                        continue;

                RecordInput(cmd);
                ParseInput(cmd, argv);
                EvaluateExpression(argv);
                input = CheckInput(argv[0]);
                
                if((!strcmp(argv[0], "exit"))  &&  argv[0])
                {
                        flag = 0;
                        continue;
                }

                switch(input)
                {
                        case builtin:
                                executeBuiltinCommand(argv);
                                break;

                        case executable:
                                executeExecutableCommand(argv);
                                break;

                        default :
                                break;
                }

        }
        while(flag);
        exit(0);
}


/*
 * Function to read user input
 * This function prompts the user for input, reads a line of input
 * from the standard input (stdin), and stores it in the provided command buffer.
 */
void ReadInput(char *cmd)
{
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        printf(ORANGE);
        printf("Nagy@Shell : ~");
        
        printf(GREEN);
        printf("%*s > ", (int)strlen(cwd) - 10, cwd + 10);
        
        printf(WHITE);
        fgets(cmd, MAX_LENGTH, stdin);
        int len = strcspn(cmd, "\n");
        cmd[len] = '\0';
}


/*
 * Function to record user input
 * This function records the user input in the 
 * command history and appends it to a log file.
 */
void RecordInput(char cmd[]) 
{
    strcpy(hist[history_count++ % MAX_HISTORY], cmd);
    FILE *file = fopen(HISTORY, "a");
    if (file) 
    {
        fprintf(file, "%s\n", cmd);
        fclose(file);
    } 
    else 
    {
        printf("Failed to Record Input Command.\n");
        return;
    }
}


/*
 * Function to parse user input into arguments
 * This function tokenizes the input command string into 
 * individual arguments and handles quoted arguments.
 */
void ParseInput(char cmd[], char *argv[])
{
    int argc = 0;
    int len = (int)strlen(cmd);
    char *index;
	
	// Tokenize the command string by space delimiter
    char *arg = strtok(cmd, " "); 
	
    // Iterate through the tokens and populate the argument array
    while (argc < MAX_ARGS && arg != NULL) 
    {
	    // Store the token as an argument
        argv[argc++] = arg;
		
		// Get the next token
        arg = strtok(NULL, " ");
        
        /* 
         * Handling Quoted Arguments. 
         * If new token (arg) isn't NULL and contains a double quote (")
        */
        if (arg && (index = strchr(arg, '"')) != NULL) 
        {
            // inserts space after the end of the current token
            *(arg + strlen(arg)) = ' ';

            // Removing Opening Quote
            memmove(index, index+1, len - (index - cmd));

            // Extract Quoted Argument
            arg = strtok(arg, "\"");
        }
    }
}


/*
 * Function to evaluate expressions in arguments
 * This function evaluates any environment variable expressions
 * in the arguments and replaces them with their corresponding values.
 */
void EvaluateExpression(char *argv[])
{
        int i;
        for(i = 1 ; argv[i] != NULL ; i++)
        {
				// Check if the argument starts with '$' indicating an environment variable expression
                if(*argv[i] == '$')
                {
						// Move to the next character after '$' and replace '\n' with null terminator to isolate the environment variable name
                        *argv[i]++ = '\n';
						
						// Get the value of the environment variable and store it in the argument

                        argv[i] = getenv(argv[i]);
                }
        }
}


/*
 * Function to check the type of input command
 * This function determines whether the input command is 
 * builtin command or an executable.
 */
input_t CheckInput(char *arg)
{
        input_t ret;
        if
        (
                (arg && (!strcmp(arg, "cd")))      || 
                (arg && (!strcmp(arg, "echoo")))    || 
                (arg && (!strcmp(arg, "history"))) || 
                (arg && (!strcmp(arg, "export")))  
        ) 
                ret = builtin;
        else 
                ret = executable;
        return ret;
}

/*
 * Function to execute an executable command
 * This function forks a new process to execute
 * the provided executable command.
*/
void executeExecutableCommand(char *argv[])
{
        pid_t child = fork();

        // Failure
        if(child < 0)
        {
                ExportError("Forking Error");
        }

        // The Child is Running
        else if(child == 0)
        {
                char *args[MAX_LENGTH] = {};
                
                // Handel Background Execution
                if(argv[1]  && (!strcmp(argv[1], "&")))
                {
						// Remove '&' from arguments
                        argv[1] = NULL;
						
						// Print child process ID for background execution
                        printf("Process : %d", getpid());
                }
				
				// Parse command arguments
                ParseArguments(args, argv);
				
				// Execute the command
                execvp(args[0], args);

                // Error Upon Return ftom exec
                ExportError("Execvp Error");
                exit(0);
        }

        // The Parent is Running
        else
        {
                // Background Execution
                // Returns without waiting for Child
                if(argv[1]  && (!strcmp(argv[1], "&")))
                        return;
					
				// Wait for the child process to finish
                waitpid(child, 0, 0);
        }

}


/*
 * Function to export an error message
 * This function prints an error message using perror()
 * and sleeps for a specified period of time.
 */ 
void ExportError(char error[])
{
        perror(error);
        usleep(SLEEP_PERIOD*1000);
}


/*
 * Function to parse command arguments
 * This function tokenizes each argument in the provided
 * argument array and stores them in a separate argument array.
 */
void ParseArguments(char *args[], char *argv[])
{
        int argc = 1;
        char *arg;
		
		// Set the first argument as the command name
        args[0] = argv[0];
		
		// Iterate through each argument in the argument array
        for(int i=1 ; i < MAX_ARGS ; i++)
        {
				// Tokenize the argument by space delimiter
                arg = strtok(argv[i], " ");
				
				// While there are still tokens in the argument
				// and there's space in the argument array
                while (argc < MAX_ARGS && arg != NULL) 
                {
						// Store the token as an argument
                        args[argc++] = arg;
						
						// Get the next token
                        arg = strtok(NULL, " ");
                }
        }
}


/*
 * Function to execute a builtin command
 * This function determines the type of builtin command
 * and executes the corresponding function.
 */
void executeBuiltinCommand(char *argv[])
{
        command_t cmd;
        cmd = CheckBuiltinCommand(argv[0]);

        switch(cmd)
        {
                case cd :
                        cdCommand(argv);
                        break;

                case echoo :
                        echoCommand(argv);
                        break;
                        
                case history :
                        historyCommand(argv);
                        break;
                        
                case export :
                        exportCommand(argv);
                        break;
        }
}


/*
 * Function to check the type of builtin command
 * This function determines the type of builtin command
 * based on the provided argument.
 */
command_t CheckBuiltinCommand(char *arg)
{
        command_t ret;

        if (arg && (!strcmp(arg, "cd")))
                ret = cd;

        else if (arg && (!strcmp(arg, "echoo")))
                ret = echoo;

        else if (arg && (!strcmp(arg, "export")))
                ret = export;

        else if (arg && (!strcmp(arg, "history")))
                ret = history;

        return ret;
}


/*
 * Function to execute the 'cd' command
 * This function changes the current working directory
 * based on the provided arguments.
 */
void cdCommand(char *argv[])
{
		// Pointer to the target directory
        char *curDirection;
        char prevDirection[MAX_LENGTH], direction[MAX_LENGTH], tempDirection[MAX_LENGTH];
		
		// Check if the argument is '~' or not provided
        if((!strcmp(argv[1], "~")) || (!argv[1]))
        {
				// Get the home directory path
                curDirection = getenv("HOME");
        }
		
		// Check if the argument is '-'
        else if(!strcmp(argv[1], "-")) 
        {
				// Set the target directory to the previous directory
                memcpy(direction, prevDirection, sizeof(prevDirection));
                curDirection = direction;
        }
        else
        {
				// Set the target directory to the provided argument
                curDirection = argv[1];
        }
		
		// Change the current working directory to the target directory
        getcwd(tempDirection, sizeof(tempDirection));

        if(chdir(curDirection))
        {
				// If changing directory fails, export an error message
                ExportError("cd");
                return;
        }
		
		// Store the current working directory as the previous directory
        memcpy(prevDirection, tempDirection, sizeof(tempDirection));
}


/*
 * Function to execute the 'echo' command
 * This function prints the arguments provided after the 'echo' command.
 */
void echoCommand(char *argv[])
{
        int i=1;
        while(argv[i])
        {
                printf("%s ", argv[i++]);
        }
        printf("\n");
}


/*
 * Function to execute the 'export' command
 * This function sets or updates environment variables
 * based on the arguments provided.
 */
void exportCommand(char *argv[])
{
        char *value, *identifier;
        int i = 1;
		
		// Iterate through each argument
        while(argv[i])
        {
				// Check if the argument contains an '=' character
                if((value = strchr(argv[i], '=')) != NULL)
                {
						// Null-terminate the string at '=' to separate identifier and value
                        *value = '\0';
						
						// Store the environment variable identifier
                        identifier = argv[i++];
						
						// Move to the value part of the argument
                        value++;
						
						// Set or update the environment variable
                        setenv(identifier, value, 1);
                }
        }
}


/*
 * Function to execute the 'history' command
 * This function prints the contents of the history file.
 */
void historyCommand(char *argv[])
{
		// Open the history file for reading
        FILE *file = fopen(HISTORY, "r");
        if(file)
        {
                size_t length;
                char *line;
				
				// Read each line from the history file and print it
                while(getline(&line, &length, file) != -1)
                        printf("%s", line);
                fclose(file);
        }
        else
        {
                ExportError("History File");
                return;
        }
}
