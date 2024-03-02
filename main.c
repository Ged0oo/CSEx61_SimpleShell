#include "main.h"

void main()     
{
        chdir(getenv("PWD"));
        runShell();
}

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
                                //executeBuiltinCommand(argv);
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

void ReadInput(char *cmd)
{
        printf("Nagy@Shell > ");
        fgets(cmd, MAX_LENGTH, stdin);
        int len = strcspn(cmd, "\n");
        cmd[len] = '\0';
}

void RecordInput(char cmd[])
{
        FILE *file;
        file = fopen(HISTORY, "a+");
        if(file)
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

void ParseInput(char cmd[], char *argv[])
{
    int argc = 0;
    int len = (int)strlen(cmd);
    char *arg = strtok(cmd, " "), *index;

    while (argc < MAX_ARGS && arg != NULL) 
    {
        argv[argc++] = arg;
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

void EvaluateExpression(char *argv[])
{
        int i;
        for(i = 1 ; argv[i] != NULL ; i++)
        {
                if(*argv[i] == '$')
                {
                        *argv[i]++ = '\n';
                        argv[i] = getenv(argv[i]);
                }
        }
}

input_t CheckInput(char *arg)
{
        input_t ret;
        if
        (
                (arg && (!strcmp(arg, "cd")))      || 
                (arg && (!strcmp(arg, "echo")))    || 
                (arg && (!strcmp(arg, "history"))) || 
                (arg && (!strcmp(arg, "export")))  
        ) 
                ret = builtin;
        else 
                ret = executable;
        return ret;
}

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
                
                // Background Execution
                if(argv[1]  && (!strcmp(argv[1], "&")))
                {
                        argv[1] = NULL;
                        printf("Process : %d", getpid());
                }

                ParseArguments(args, argv);
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
                waitpid(child, 0, 0);
        }

}

void ExportError(char error[])
{
        perror(error);
        usleep(SLEEP_PERIOD*1000);
}

void ParseArguments(char *args[], char *argv[])
{
        int argc = 1;
        char *arg;
        args[0] = argv[0];

        for(int i=1 ; i < MAX_ARGS ; i++)
        {
                arg = strtok(argv[i], " ");

                while (argc < MAX_ARGS && arg != NULL) 
                {
                        args[argc++] = arg;
                        arg = strtok(NULL, " ");
                }
        }
}