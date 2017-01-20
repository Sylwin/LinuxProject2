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

struct sockaddr_in publicChannel;
int numberOfWorkers;
char message[50];
timer_t intervalTimerId;
struct itimerspec sendMsgTimer;
pid_t groupLeaderPID;
int fd;
char *brygadaChannel = "brygadaChannel";

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
    //printf("lol\n");
}

int main(int argc, char* argv[])
{
    int opt;
    float time = 0;
    char registerChannelName[50];
    struct sockaddr_un name;
    int data_socket;
    int ret;
    pid_t f;
    pid_t grpPid;
    int grp;

    while( (opt = getopt(argc, argv, "a:n:c:t:")) != -1 )
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
        case '?':
        default:
            fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
            break;
        }
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
   // printf("Message: %s\n", message);
   // printf("Time: %ld, %ld\n", sendMsgTimer.it_value.tv_sec, sendMsgTimer.it_value.tv_nsec);
   // printf("Number of workers: %d\n", numberOfWorkers);
   // printf("Address: %ud\n", publicChannel.sin_addr.s_addr);
//----------------------------------------------------
    //mkfifo(brygadaChannel, 0666);
    fd = open(brygadaChannel, O_WRONLY | O_NONBLOCK);
    write(fd, message, sizeof(message));
//----------------------------------------------------
    //publiczny socket Brygadzista <-> Archiwista
    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(data_socket == -1)
        error("socket");

    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, registerChannelName, sizeof(name.sun_path)-1);

    ret = connect(data_socket, (const struct sockaddr *)&name, sizeof(struct sockaddr_un));
    if(ret == -1)
        error("connect");

    ret = write(data_socket, message, strlen(message)+1);
    if(ret==-1)
        error("write");
//----------------------------------------------------

    groupLeaderPID = fork();
    printf("groupLeader PID: %d\n", groupLeaderPID);

    if(groupLeaderPID == -1)
        error("fork1");
    else if(groupLeaderPID == 0)
    {
        grp = setpgrp();
        if(grp == -1)
            error("setpgrp");

        for(int i = 0; i < numberOfWorkers; i++)
        {
            //tworzymy robotnikow
            char *newArgs[] = { (char *) 0 };
            if( fork() == 0)
            {
                //printf("dupa\n");
                execvp("./robotnik.o", newArgs);
                //grpPid = getpgrp();
                //printf("[son] pid %d from pid %d\n", getpid(), grpPid);
                //printf("dupa\n");
                exit(1);
            }
        }
        exit(1);
    }
    else
    {
        printf("brygadzista pid: %d\n", getpid());
    }
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
   // close(fd);
   // unlink(brygadaChannel);
    return 0;
}

