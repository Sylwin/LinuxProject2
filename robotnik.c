#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <time.h>

struct timespec sendTime;
int ret;
int sockfd;

#define SOCKET "/tmp/socks"

void error(const char* msg)
{
    perror(msg);
    exit(0);
}

void sigHandler(int sig)
{
       char buffer;
    if( read(0, &buffer, 1) == -1 )
        error("read");
    //printf("Received: %s\n", &buffer);
    clock_gettime(CLOCK_REALTIME, &sendTime);
    char msg[20];
    sprintf(msg, "%c %ld.%9ld", buffer, sendTime.tv_sec, sendTime.tv_nsec);
    printf("%s\n", msg);
//    ret = write(sockfd, msg, sizeof(msg));
//    if( ret == -1 )
//        error("write");
}

int main(int argc, char* argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigHandler;
    sigemptyset(&sa.sa_mask);
    if( sigaction(SIGALRM, &sa, NULL) == -1 )
        perror("sigaction");

    unlink(SOCKET);
    struct sockaddr_un address;
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == 1)
        error("socket");

    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET);

//    ret = connect(sockfd, (const struct sockaddr*)&address, sizeof(struct sockaddr_un));
//    if( ret == -1 )
//        error("connect");

    while(1)
        pause();

    return 0;
}


