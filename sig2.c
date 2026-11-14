#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <signal.h>



void signal_handler(int sig) {

    if (sig == SIGINT) {

        printf("\nThis is a special signal handler for SIGINT\n");

    } else if (sig == SIGTSTP) {

        printf("\nThis is a special signal handler for SIGTSTP\n");

    } else if (sig == SIGQUIT) {

        printf("\nThis is a special signal handler for SIGQUIT\n");

    }

}



int main(void) {

    if (signal(SIGINT, signal_handler) == SIG_ERR ||

        signal(SIGTSTP, signal_handler) == SIG_ERR ||

        signal(SIGQUIT, signal_handler) == SIG_ERR) {

        perror("signal");

        exit(1);

    }



    printf("Running... Press Ctrl+C (SIGINT), Ctrl+Z (SIGTSTP), or Ctrl+\\ (SIGQUIT).\n");

    printf("Press Ctrl+D to exit.\n");



    char s[200];

    while (fgets(s, sizeof(s), stdin) != NULL) {

        // Keep looping so you can test all 3 signals in one run

    }



    return 0;

}
