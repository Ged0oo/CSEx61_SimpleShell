#ifndef __MAIN_H__
#define __MAIN_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>


#define MAX_LENGTH      256
#define MAX_ARGS        256
#define SLEEP_PERIOD    200
#define LOGS            "/home/nagy/Desktop/nagy/SimpleShell/shellLOGS.log"
#define HISTORY         "/home/nagy/Desktop/nagy/SimpleShell/shellHISTORY"

#define ORANGE          "\033[0;33m"
#define GREEN           "\033[0;32m"
#define WHITE           "\033[0m"

typedef enum
{
        builtin,
        executable
}input_t;


typedef enum
{
        cd,
        echo,
        export,
        history
}command_t;


void runShell();
void executeBuiltinCommand(char *argv[]);
void executeExecutableCommand(char *argv[]);

void ReadInput(char *cmd);
void RecordInput(char cmd[]);
void ParseInput(char cmd[], char *argv[]);
void EvaluateExpression(char *argv[]);
input_t CheckInput(char *arg);

void ExportError(char error[]);
void ParseArguments(char *args[], char *argv[]);
command_t CheckBuiltinCommand(char *arg);

void cdCommand(char *argv[]);
void echoCommand(char *argv[]);
void exportCommand(char *argv[]);
void historyCommand(char *argv[]);

#endif //__MAIN_H__












