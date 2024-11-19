// reads integers from the keyboard until EOF
// create two child processes and connect the children and parent in a ring using pipes
    // the first child will square the received value and forward the result to the other child
    // the second child will add one to the received value and return it to the parent
    // the parent will print the new value to the screen and read another integer
// when EOF is reached, the parent must close its pipes and wait for its children to properly exit.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    // pipe(int fd[2]) - fd[0] is open for reading - fd[1] is open for writing
        // output of fd[1] is the input for fd[0]    
    // CREATE PIPES: fd1 (parent → first child), fd2 (first child → second child), fd3 (second child → parent)
    int fd1[2], fd2[2], fd3[2];
    pid_t pid1, pid2;

    if (pipe(fd1) == -1 || pipe(fd2) == -1 || pipe(fd3) == -1) {
        perror("ERROR: pipe failed");
        return 1;
    }

    // FORK FIRST CHILD PROCESS (square)
    pid1 = fork();

    if (pid1 < 0) {
        perror("ERROR: fork failed");
        return 1;
    } else if (pid1 == 0) {
        // FIRST CHILD PROCESS

        // close unused read/write end of the pipe
        close(fd1[1]);  // close write end of fd1
        close(fd2[0]);  // close read end of fd2
        close(fd3[0]);  // close read end of fd3
        close(fd3[1]);  // close write end of fd3
        
        // SQUARE THE VALUE AND PASS IT ON
        int n;
        while (read(fd1[0], &n, sizeof(int)) > 0) {
            n = n * n;
            write(fd2[1], &n, sizeof(int));
        }

        // CLOSE THE REMAINING PIPES AND EXIT
        close(fd1[0]);  // read end of fd1: parent → first child
        close(fd2[1]);  //write end of fd2: first child → second child
        exit(EXIT_SUCCESS);
    }

    // FORK SECOND CHILD PROCESS (add 1)
    pid2 = fork();

    if (pid2 < 0) {
        perror("ERROR: fork failed");
        return 1;
    } else if (pid2 == 0) {
        // SECOND CHILD PROCESS

        // close unused read/write end of the pipe
        close(fd1[0]);  // close read end of fd1
        close(fd1[1]);  // close write end of fd1
        close(fd2[1]);  // close write end of fd2
        close(fd3[0]);  // close read end of fd3

        // ADD ONE TO THE VALUE AND PASS IT ON
        int n;
        while (read(fd2[0], &n, sizeof(int)) > 0) {
            n = n + 1;
            write(fd3[1], &n, sizeof(int));  
        }

        // CLOSE THE REMAINING PIPES AND EXIT
        close(fd2[0]);  // read end of fd2: first child → second child
        close(fd3[1]);  // write end of fd3: second child → parent
        exit(EXIT_SUCCESS);
    }

    // PARENT PROCESS (printing)

    // close unused read/write end of the pipe
    close(fd1[0]);  // close read end of fd1
    close(fd2[0]);  // close read end of fd2
    close(fd2[1]);  // close write end of fd2
    close(fd3[1]);  // close write end of fd3
    
    // PRINT THE FINAL PIPED AND PROCESSED NUMBER VALUE
    int n;
    while (scanf("%d", &n) != EOF) {
        write(fd1[1], &n, sizeof(int));
        read(fd3[0], &n, sizeof(int));

        printf("Final Processed Value: %d\n", n);
    }

    // CLOSE THE REMAINING PIPES AND WAIT FOR CHILDREN TO PROPERLY EXIT
    close(fd1[1]);  // write end of fd1: parent → first child
    close(fd3[0]);  // read end of fd3: second child → parent

    wait(NULL);
    wait(NULL);

    return 0;
}
