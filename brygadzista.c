#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/un.h>

#define BUFFER_SIZE 12

struct sockaddr_in publicChannel;
int numberOfWorkers;
char message[50];

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    int opt;
    struct itimerspec sendMsgTimer;
    float time = 0;
    char socketName[50];
    struct sockaddr_un name;
    int data_socket;
    int ret;

    while( (opt = getopt(argc, argv, ":a:n:c:t:")) != -1 )
    {
        switch(opt)
        {
        case 'a':
            strcpy(socketName, optarg);
            break;
        case 'n':
            numberOfWorkers = strtof(optarg,NULL);
            break;
        case 'c':
            strcpy(message, optarg);
            break;
        case 't':
            time = strtof(optarg, NULL);
            sendMsgTimer.it_value.tv_sec = floor(time);
            sendMsgTimer.it_value.tv_nsec = (time - floor(time))*1000000000;
            break;
        case ':':
            fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[0], optopt);
            break;
        case '?':
        default:
            fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
            break;
        }
    }

   // printf("Message: %s\n", message);
   // printf("Time: %ld, %ld\n", sendMsgTimer.it_value.tv_sec, sendMsgTimer.it_value.tv_nsec);
   // printf("Number of workers: %d\n", numberOfWorkers);
   // printf("Address: %ud\n", publicChannel.sin_addr.s_addr);

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(data_socket == -1)
        error("socket");

    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, socketName, sizeof(name.sun_path)-1);

    ret = connect(data_socket, (const struct sockaddr *)&name, sizeof(struct sockaddr_un));
    if(ret == -1)
        error("connect");

   // strcpy(buffer, "hello world");

    ret = write(data_socket, message, strlen(message)+1);
    if(ret==-1)
        error("write");

    return 0;
}

