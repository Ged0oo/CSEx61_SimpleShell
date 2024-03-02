#ifndef __MAIN_H__
#define __MAIN_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


#define MAX_LENGTH      256
#define MAX_ARGS        256
#define SLEEP_PERIOD    200
#define LOGS            "/home/nagy/Desktop/nagy/SimpleShell/shellLOGS.log"
#define HISTORY         "/home/nagy/Desktop/nagy/SimpleShell/shellHISTORY"


typedef enum
{
        builtin,
        executable
}input_t;


typedef enum
{
        export,
        history
}command_t;


void runShell();
void executeBuiltinCommand();
void executeExecutableCommand();

void ReadInput(char *cmd);
void RecordInput(char cmd[]);
void ParseInput(char cmd[], char *argv[]);
void EvaluateExpression(char *argv[]);
input_t CheckInput(char *argv);

#endif //__MAIN_H__












