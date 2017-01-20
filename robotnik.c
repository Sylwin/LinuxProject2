#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

int fd;
char *brygadaChannel = "brygadaChannel";
char buffer[256];

void sigHandler(int sig)
{
    printf("yo\n");
    //wysylaie jednego bajta do archiwisty
    read(fd, buffer, 1);
    printf("Received: %s\n", buffer);
}

int main(int argc, char* argv[])
{
    fd = open(brygadaChannel, O_RDONLY | O_NONBLOCK);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigHandler;
    sigemptyset(&sa.sa_mask);
    if( sigaction(SIGALRM, &sa, NULL) == -1 )
        perror("sigaction");

    while(1)
        pause();

   // close(fd);
    return 0;
}
