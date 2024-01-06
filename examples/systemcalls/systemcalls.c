#include "systemcalls.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
  int returnStatus= system(cmd);
  if (returnStatus == 0){
  return true; 
   }
   else{
   return false;
   }
   
   
   
   // return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
     bool result;
    pid_t pid = fork();

    if (pid == -1) {
    printf("Fork failed %s\n",strerror(errno));
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
    
     for(i=0; i<count; i++)
    {
     printf("CMD : %s\n",command[i]);
        
    }
     fflush(stdout);
     
        if(execv(command[0], command) ==-1){
        perror("Exec failed");
        exit(EXIT_FAILURE);
        }
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
        
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status == 0) {
                printf("Command executed successfully!\n");
                result=true;
            } else {
                printf("Command execution failed with status: %d\n", exit_status);
                fflush(stdout);
                
            }
        } else {
            printf("Command execution failed\n");
        fflush(stdout);
                        result= false;
        }
        
    }
    va_end(args);

    return result;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
pid_t pid; 


int fd = open(outputfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
if (fd <0 ) { 
    perror("open"); // Print an error message if file opening fails
    abort(); // Abort the program if file opening fails
}

pid = fork(); // Fork a child process
if (pid <0) {
    printf("Fork failed with %s\n", strerror(errno)); 
    fflush(stdout); // Flush stdout to ensure the message is displayed
    return false;
}

if (pid == 0) { // Child process's code

    // Output the command strings before redirection
    for (i = 0; i < count; i++) {
        printf("cmd: %s\n", command[i]);
    }
    fflush(stdout); // Flush stdout to ensure the messages are displayed

    // Redirect stdout to the file descriptor obtained earlier
    if (dup2(fd, 1) < 0) {
        perror("dup2"); 
        abort(); 
    }
    close(fd); 


    int ret = execv(command[0], command);
    if (ret == -1) {
        perror("child execv failed"); 
        return false; 
    }
} else { // Parent process's code
    close(fd); 

    int status;
    // Wait for the child process to terminate and retrieve its status
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid() failed"); 
        return false; 
    }

    int returned = 0;
    int signum = 0;

    if (WIFEXITED(status)) {
        returned = WEXITSTATUS(status);
        printf("at do_exec_redirect, Exited normally with retCode %d\n", returned);
    } else if (WIFSIGNALED(status)) {
        signum = WTERMSIG(status);
        printf("Exited due to receiving signal %d\n", signum);
    } else if (WIFSTOPPED(status)) {
        signum = WSTOPSIG(status);
        printf("Stopped due to receiving signal %d\n", signum);
    } else {
        printf("Something strange just happened.\n");
    }

    printf("status %d\n", status);
    printf("Parent received child %d exit status was %d OR signum %d \n", pid, returned, signum);
    fflush(stdout);
    return returned == 0;
}

printf("pid %d at do_exec_redirect end\n", pid); 
fflush(stdout); // Flush stdout to ensure the message is displayed

    va_end(args);

    return true;
}
//commit
