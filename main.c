#include "main.h"

void main()     
{
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
                RecordInput(cmd);
                ParseInput(cmd, argv);
                
                for (int i = 0; argv[i] != NULL; i++)
                        printf("Argument %d: %s\n", i, argv[i]);

                //EvaluateExpression(argv);
                //input = CheckInput(argv[0]);

                if((!strcmp(argv[0], "exit"))  &&  argv[0])
                {
                        flag = 0;
                        continue;
                }

                switch(input)
                {
                        case builtin:
                                //executeBuiltinCommand();
                                break;

                        case executable:
                                //executeExecutableCommand();
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