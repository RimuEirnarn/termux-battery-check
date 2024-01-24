#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "common.h"

int spawn_out(const char *filename,
        argvT argv,
        string result) {
    int pipe_fd[2];
    pid_t child_pid;
    
    // Create a pipe for communication
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        return -1;
    }

    // Fork a child process
    if ((child_pid = fork()) == -1) {
        perror("Fork failed");
        return -1;
    }

    if (child_pid == 0) {  // Child process
        // Redirect stdout to the write end of the pipe
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            perror("Error");
            _exit(EXIT_FAILURE);
        };
        //close(pipe_fd[0]); 
        //close(pipe_fd[1]);

        // Execute the termux-battery-status command
        execvp(filename, argv);
        perror("Exec failed");
        _exit(EXIT_FAILURE);
    }
    wait(&child_pid);
    close(pipe_fd[1]);

    FILE *out = fdopen(pipe_fd[0], "r");
    if (out == NULL) {
        perror("Error opening pipe for reading");
        return -1;
    }

    //while (fgets(result, sizeof(result), out) != NULL);
    ssize_t bytesr = read(pipe_fd[0], result.data, result.size);
    if (bytesr == -1) {
        perror("Errr opening pipe for reading");
        return -1;
    }
    
    result.data[bytesr] = '\0';
    fclose(out);
    return 0;
}

int spawn(const char* filename,
        argvT argv,
        const string input,
        string output) {
    int pipe_input[2];
    int pipe_output[2];
    pid_t child_pid;

    // Create pipes for communication
    if (pipe(pipe_input) == -1 || pipe(pipe_output) == -1) {
        perror("Pipe creation failed");
        return -1;
    }

    // Fork a child process
    if ((child_pid = fork()) == -1) {
        perror("Fork failed");
        return -1;
    }

    if (child_pid == 0) {  // Child process
        // Redirect stdin to the read end of the input pipe
        dup2(pipe_input[0], STDIN_FILENO);
        close(pipe_input[0]);
        close(pipe_input[1]);

        // Redirect stdout to the write end of the output pipe
        dup2(pipe_output[1], STDOUT_FILENO);
        close(pipe_output[0]);
        close(pipe_output[1]);

        // Execute the specified command with a single argument
        execvp(filename, argv);
        perror("Exec failed");
        _exit(EXIT_FAILURE);
    } else {  // Parent process
        close(pipe_input[0]);
        close(pipe_output[1]);

        // Write the input to the input pipe
        write(pipe_input[1],
                input.data, input.size);
        close(pipe_input[1]);
        wait(NULL);

        // Read the output from the output pipe
        FILE *out = fdopen(pipe_output[0], "r");
        if (out == NULL) {
            perror("Error opening pipe for reading");
            return -1;
        }

        // Read the content from the pipe
        fgets(output.data, output.size, out);

        fclose(out);
        close(pipe_output[0]);

        // Wait for the child process to finish
    }

    return 0;
}
