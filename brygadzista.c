#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/un.h>
#include <signal.h>

int numberOfWorkers;
char message[50];
timer_t intervalTimerId;
struct itimerspec sendMsgTimer;
pid_t groupLeaderPID;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void sigHandler(int sig)
{
    if( timer_settime(intervalTimerId, 0, &sendMsgTimer, NULL) == -1 )
        perror("timer_settime3");
    killpg(groupLeaderPID, SIGALRM);
}

int main(int argc, char* argv[])
{
    int opt;
    float time = 0;
    char registerChannelName[50];
    int sockfd;
    int ret;
    int grp;
    char id[20];

    while( (opt = getopt(argc, argv, "a:n:c:t:i:")) != -1 )
    {
        switch(opt)
        {
        case 'a':
            strcpy(registerChannelName, optarg);
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
        case 'i':
            strcpy(id, optarg);
            break;
        case '?':
        default:
            fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
            break;
        }
    }

    if( argc < 5 )
    {
        printf("Usage: ./brygadzista -aSTRING -nINT -cSTRING -tFLOAT\n");
        exit(0);
    }

//----------------------------------------------------
    //publiczny socket rejestrujacy
    struct sockaddr_un name;
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1)
        error("socket");
    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, registerChannelName);

    ret = connect(sockfd, (const struct sockaddr *)&name, sizeof(struct sockaddr_un));
    if(ret == -1)
        error("connect1");

    ret = write(sockfd, id, sizeof(id)+1);
    if(ret==-1)
        error("write");
    close(sockfd);
//----------------------------------------------------
    struct sockaddr_un brygArch;
    memset(&brygArch, 0, sizeof(struct sockaddr_un));
    brygArch.sun_family = AF_UNIX;
    strcpy(brygArch.sun_path, id);
    printf("brygada id: %s\n", id);
    int new = socket(AF_UNIX, SOCK_STREAM, 0);
    if(new == -1)
        error("socket");
    if(connect(new, (const struct sockaddr *)&brygArch, sizeof(struct sockaddr_un)) == -1)
        error("connect2");
    int tmp = htonl(numberOfWorkers);
    if( write(new, &numberOfWorkers, 1) == -1 )
        error("write");

//----------------------------------------------------

    int fd[2];
    pipe(fd);

    groupLeaderPID = fork();
    printf("groupLeader PID: %d\n", groupLeaderPID);

    if(groupLeaderPID == -1)
        error("fork1");
    else if(groupLeaderPID == 0)
    {
        grp = setpgrp();
        if(grp == -1)
            error("setpgrp");

        for(int i = 1; i <= numberOfWorkers; i++)
        {
            //tworzymy robotnikow
            char socket[20];
            sprintf(socket, "-nsocket%d", i);
            char *newArgs[] = { "./robotnik.o", socket, (char *) 0 };
            if( fork() == 0)
            {
                dup2(fd[0],0);
                close(fd[1]);
                execvp("./robotnik.o", newArgs);
                exit(1);
            }
        }
        exit(1);
    }
    else
    {
        printf("brygadzista pid: %d\n", getpid());
        write(fd[1], message, sizeof(message)+1);
    }
//----------------------------------------------------

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigHandler;
    sigemptyset(&sa.sa_mask);
    if( sigaction(SIGALRM, &sa, NULL) == -1 )
        perror("sigaction");

    struct sigevent se;
    memset(&se, 0, sizeof(se));
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;
    se.sigev_value.sival_ptr = &intervalTimerId;
    if( timer_create(CLOCK_REALTIME, &se, &intervalTimerId) == -1 )
        perror("timer_create1");

    if( timer_settime(intervalTimerId, 0, &sendMsgTimer, NULL) == -1 )
        perror("timer_settime1");

//----------------------------------------------------

    while(1)
    {
        if( kill(getpid(), 0) == ESRCH )
        {
            killpg(groupLeaderPID, SIGTERM);
            exit(0);
        }
        pause();
    }

    return 0;
}
