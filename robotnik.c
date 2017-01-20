#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

void sigHandler(int sig)
{
    printf("yo\n");
    //wysylaie jednego bajta do archiwisty
}

int main(int argc, char* argv[])
{

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigHandler;
    sigemptyset(&sa.sa_mask);
    if( sigaction(SIGALRM, &sa, NULL) == -1 )
        perror("sigaction");

    while(1)
        pause();

    return 0;
}
