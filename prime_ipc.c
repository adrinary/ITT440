#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void sigint_handler(int sig) {
    printf("\n[Signal Handler] Caught SIGINT (Ctrl+C)! Exiting safely...\n");
    exit(0);
}

int isPrime(int n) {
    if (n <= 1) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main(void) {
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(fd[0]);
        int num;
        printf("Child (PID=%d) - Enter a positive integer: ", getpid());
        fflush(stdout);
        if (scanf("%d", &num) != 1) {
            printf("Invalid input.\n");
            close(fd[1]);
            exit(EXIT_FAILURE);
        }
        write(fd[1], &num, sizeof(num));
        close(fd[1]);
        exit(EXIT_SUCCESS);
    } else {
        close(fd[1]);
        wait(NULL);

        int receivedNum;
        read(fd[0], &receivedNum, sizeof(receivedNum));
        close(fd[0]);

        if (isPrime(receivedNum)) {
            printf("Parent (PID=%d) - %d IS a prime number.\n", getpid(), receivedNum);
        } else {
            printf("Parent (PID=%d) - %d is NOT a prime number.\n", getpid(), receivedNum);
        }
    }

    return EXIT_SUCCESS;
}
