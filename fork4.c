#include <stdio.h>

#include <stdlib.h>

#include <sys/types.h>

#include <sys/wait.h>

#include <unistd.h>



void childTask(int childNum) {

    char name[50];

    printf("Child %d (PID=%d) - Enter your name: ", childNum, getpid());

    scanf("%49s", name);

    printf("Child %d (PID=%d) - Your name is %s\n\n", childNum, getpid(), name);

}



int main(void) {

    for (int i = 1; i <= 4; i++) {

        pid_t pid = fork();



        if (pid == 0) {

            childTask(i);

            exit(EXIT_SUCCESS);

        } else if (pid > 0) {

            wait(NULL);

        } else {

            perror("fork");

            exit(EXIT_FAILURE);

        }

    }



    printf("Job is done\n");

    return EXIT_SUCCESS;

}
