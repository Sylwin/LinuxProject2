#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/un.h>

#define BUFFER_SIZE 256

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

struct sockaddr_in publicChannel;

int main(int argc, char* argv[])
{
    int sockfd;
    struct sockaddr_un name;
    int result;
    char buffer[BUFFER_SIZE];
    int opt;
    char socketName[50];
    int data_socket;
    int ret;

    while( (opt = getopt(argc, argv, "a:")) != -1 )
    {
        switch(opt)
        {
        case 'a':
            strcpy(socketName, optarg);
            break;
        case '?':
        default:
            fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
            break;
        }
    }

    unlink(socketName);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if( sockfd == -1 )
        error("socket");

    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, socketName, sizeof(name.sun_path)-1);

    if( bind(sockfd, (struct sockaddr *)&name, sizeof(struct sockaddr_un)) == -1 )
        error("bind");

    listen(sockfd, 10);
    while(1)
    {
        data_socket = accept(sockfd, NULL, NULL);
        if( data_socket == -1 )
            error("accept");

        ret = read(data_socket, buffer, BUFFER_SIZE);
        if(ret==-1)
            error("read");

        printf("Here is the message: %s\n", buffer);
    }

    return 0;
}
