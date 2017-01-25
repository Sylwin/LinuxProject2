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
#include <poll.h>
#include <openssl/md5.h>

struct message
{
    char value;
    long int sec;
    long int nsec;
};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int compare(const void* left, const void* right)
{
    struct message l = *((struct message*)left);
    struct message r = *((struct message*)right);
    if(l.sec > r.sec) return 1;
    else if(l.sec < r.sec) return -1;
    if(l.nsec > r.nsec) return 1;
    else if(l.nsec < r.nsec) return -1;
    return 0;
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

    listen(sockfd, 5);
    registerSocket = accept(sockfd, NULL, NULL);
    if( registerSocket == -1 )
        error("accept");

    ret = read(registerSocket, buffer, sizeof(buffer));
    if(ret==-1)
        error("read");

    int len = strlen(buffer);

//------------------------------------------------------------
    char receiveMD5sum[100];
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

    listen(brygadzistaSock, 5);

    int data = accept(brygadzistaSock, NULL, NULL);
    if( data == -1 )
        error("accept");

    int number = 0;
    read(data, receiveMD5sum, sizeof(receiveMD5sum));
    //printf("md5sum: %s\n", receiveMD5sum);
    number = receiveMD5sum[0];
    //printf("number: %c\n", number);
    char md5sum[100];
    strcpy(md5sum, receiveMD5sum + 1 );
    //printf("md5sum: %s\n",md5sum);

//------------------------------------------------------------

    int socks[20];
    struct pollfd fds[20];
    fds[0].fd = brygadzistaSock;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    int res;
    int workers = number;
    for(int i = 0; i < number; i++)
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

        fds[i].fd = socks[i];
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }
    int z = 0;
    char *Message = malloc(number*sizeof(char));
    memset(Message, 0, number*sizeof(char));
    struct message *ms = malloc(sizeof(struct message)*number);

    while(1)
    {
        res = poll(fds, number, -1);
        for(int i = 0; i < number; i++)
        {
            if(fds[i].revents & POLLIN)
            {
                char msg[40];
                if( read(socks[i], msg, sizeof(msg)) == -1 )
                    error("read");
                if(strlen(msg) != 0)
                {
                    //printf("receive: %s\n", msg);
                    ms[z].value = msg[0];
                    char *second;
                    ms[z].sec = strtol(msg+1, &second, 10);
                    ms[z].nsec = strtol(second+1, NULL, 10);
                    z++;
                }
            }
            else if( (fds[i].revents & POLLNVAL) & (fds[i].revents & POLLERR) & (fds[i].revents & POLLHUP) )
                break;
        }

        if( z == len )
        {
            qsort(ms, z, sizeof(struct message), compare);
            for( int i = 0; i < z; i++ )
            {
                Message[i] = ms[i].value;
            }
            printf("get message: %s\n", Message);
            z=0;
            char out[MD5_DIGEST_LENGTH];
            char *res = MD5(Message, strlen(Message), out);
            char *newMd5sum = malloc(100);
            memset(newMd5sum,0,100);
            for(int n=0; n<MD5_DIGEST_LENGTH; n++)
                sprintf(newMd5sum,"%s%02x",newMd5sum,  out[n]);
            //printf("md5sum: %s\n", newMd5sum);
            if( strcmp(md5sum,newMd5sum) == 0 )
                printf("message is correct!\n");
            else
                printf("Something went wrong!\n");
        }
    }

    return 0;
}

