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
#include <netinet/udp.h>


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    int sockfd;
    char buffer[20];
    int opt;
    char registerChannelName[50];
    int registerSocket;
    int ret;

    while( (opt = getopt(argc, argv, "a:")) != -1 )
    {
        switch(opt)
        {
        case 'a':
            strcpy(registerChannelName, optarg);
            break;
        case '?':
        default:
            fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
            break;
        }
    }

    if( argc < 2 )
    {
        printf("Usage: ./archiwista.o -aSTRING\n");
        exit(0);
    }
//------------------------------------------------------------
    unlink(registerChannelName);
    struct sockaddr_un name;
    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, registerChannelName, sizeof(name.sun_path)-1);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if( sockfd == -1 )
        error("socket");

    if( bind(sockfd, (struct sockaddr *)&name, sizeof(struct sockaddr_un)) == -1 )
        error("bind");

    listen(sockfd, 10);
//------------------------------------------------------------
    int socks[20];

    while(1)
    {
        registerSocket = accept(sockfd, NULL, NULL);
        if( registerSocket == -1 )
            error("accept");

        ret = read(registerSocket, buffer, sizeof(buffer));
        if(ret==-1)
            error("read");
        printf("brygada: %s\n", buffer);

        unlink(buffer);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, buffer);

        int brygadzistaSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if( brygadzistaSock == -1)
            error("brygadzistaSock");
        if(bind(brygadzistaSock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
            error("brygadzistaBind");
         printf("%s\n", buffer);

        listen(brygadzistaSock, 5);

        int data = accept(brygadzistaSock, NULL, NULL);
        if( data == -1 )
            error("accept");
        int number = 0;
        read(data, &number, 1);
        printf("number of workers: %d\n", number);

        int workers = number;
        for(int i = 1; i <= number; i++)
        {
            char path[20];
            struct sockaddr_un workerAddr;
            memset(&workerAddr, 0, sizeof(struct sockaddr_un));
            workerAddr.sun_family = AF_UNIX;
            sprintf(path, "socket%d", workers--);
            strcpy(workerAddr.sun_path, path);
            socks[i] = socket(AF_UNIX, SOCK_DGRAM, 0);
            unlink(path);

            if(socks[i] == -1)
                error("socks");
            if( bind( socks[i], (struct sockaddr *)&workerAddr, sizeof(struct sockaddr_un)) == -1 )
                error("binds");
            printf("bind with: %s\n", path);
            char msg[50];
            if( read(socks[i], msg, 1) == -1 )
                error("receive");
            printf("receive: %s\n", msg);
        }

    }

    return 0;
}

