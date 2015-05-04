/*
    client.h
*/

#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>


// FIFO write operation are atomic only using at most 1024 bytes
#define MAX_BUF 1024
#define NAME_LEN 11
#define SERVER_PATH "/tmp/hairy-happiness"
#define DELIM " "
#define MSG_QUESTION "q"
#define MSG_ACCEPTED "accepted"
#define MSG_NOTACCEPTED "nope"
#define MSG_JOIN "join"
#define MSG_ANSWER "answer"
#define MSG_INCORRECT "no"
#define MSG_CORRECT "yes"
#define MSG_END "end"
#define MSG_QUIT "quit"


#define ERR_NOTACCSF -1
#define ERR_NOTACCAE -2

char * toString ( const char * format, ... )
{
    char buffer[MAX_BUF];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    //perror (buffer);
    va_end (args);
    return strdup(buffer);
}


/* 
    manage the user interaction with the game
*/
void read_fifo(void* args)
{
    int recvFIFO = *((int*) args);

    while(1)
    {
        // read message
        char buf[MAX_BUF];
        read(recvFIFO, buf, MAX_BUF);

        // extract the op
        char * data;
        char * op = strtok_r(buf, DELIM, &data);
        if (!strcmp(op, MSG_QUESTION))
        {
            write(1, strtok_r(NULL, DELIM, &data), MAX_BUF);
        }
        else if (!strcmp(op, MSG_CORRECT))
        {
            printf("correct\n");
        }
        else if (!strcmp(op, MSG_INCORRECT))
        {
            printf("wrong\n");
        }
        else if (!strcmp(op, MSG_END))
        {
            printf("end\n");
        }
        else if (!strcmp(op, MSG_QUIT))
        {
            printf("someone has quit\n");
        }
        else if (!strcmp(op, MSG_JOIN))
        {
            printf("someone joined\n");
        }
    }
}


void read_console(int sendFIFO, int index)
{
    while (1)
    {
        // wait for the user to answer
        char input[MAX_BUF];
        size_t len = read(0, input, MAX_BUF);
        input[len] = '\0';

        if (!strcmp(input, "quit\n"))
        {
            write(sendFIFO, toString("%s%s%d", MSG_QUIT, DELIM, index), MAX_BUF);
            break;
        }
        else
        {
            errno = 0;
            // convert the number
            long answer = strtol(input, NULL, 10);

            if (errno != 0 && answer == 0)
            {
                // conversion error
                printf("Insert a number please\n");
            }
            else // send the user answer
            {
                write(sendFIFO, toString("%s%s%d%s%ld", MSG_ANSWER, DELIM, index, DELIM, answer), MAX_BUF);
            }
        }
    }
}



int main()
{
    // get user nickname
    char name[NAME_LEN];
    printf("Enter your nickname: ");
    scanf("%10s", name);
    

    // open the output FIFO with write permissions and wait for the server to exist
    int sendFIFO = open(SERVER_PATH, O_WRONLY);
    
    
    char client_path[30];
    // generate fifo name using timestamp  FIXME: timestamp has seconds precision
    sprintf(client_path, "/tmp/%u", (unsigned)time(NULL));
    // create the FIFO
    mkfifo(client_path, 0666);    // open the input FIFO with read permissions (blocking)
    int recvFIFO = open(client_path, O_RDWR);
    
    
    
    //write join request
    write(sendFIFO, toString("%s%s%s%s%s", MSG_JOIN, DELIM, client_path, DELIM, name), MAX_BUF);
    // read the answer to the join request
    char buf[MAX_BUF];
    size_t len = read(recvFIFO, buf, MAX_BUF);
    buf[len] = '\0';

    char * op = strtok(buf, DELIM);
    
    if (!strcmp(op, MSG_NOTACCEPTED))
    {
        int err = atoi(strtok(NULL, DELIM));
        if(err == ERR_NOTACCAE)
            printf("The server didn't accepted you, your name already exist\n");
        else if (err == ERR_NOTACCSF)
            printf("The server is full\n");
    }
    else if (!strcmp(op, MSG_ACCEPTED))
    {
        printf("Joined to server\n");
        
        // get the index of this client on the server
        int index = atoi(strtok(NULL, DELIM));
        
        // create thread for listening to the server
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, (void*)read_fifo, (void*)&recvFIFO);

        // keep sending user answer
        read_console(sendFIFO, index);
    }

    close(recvFIFO);
    unlink(client_path);

    return 0;
}
