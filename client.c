/*
    client.h
*/

#include <fcntl.h>
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


/*
    struct for passing argumnets in pthread_create
*/
typedef struct thread_args {
    int in_f;
} thread_args;


/*
    send join request to server
*/
void join_game(int out_f, char * name, char * client_path) {
    char buf[MAX_BUF];
    sprintf(buf, "%s%s%s%s%s", MSG_JOIN, DELIM, client_path, DELIM, name);
    write(out_f, buf, MAX_BUF);
}


/* 
    manage the user interaction with the game
*/
void read_fifo(void* args) {
    thread_args c = *((thread_args*) args);

    while(1) {
        // read message
        char buf[MAX_BUF];
        read(c.in_f, buf, MAX_BUF);

        // extract the op
        char * buf_dup = strdup(buf);
        char * op = strtok(buf, DELIM);
        if (!strcmp(op, MSG_QUESTION)) {
            // obtain only the question
            buf_dup += 2;
            write(1, buf_dup, strlen(buf_dup));
        }
        else if (!strcmp(op, MSG_CORRECT)) {
            printf("correct\n");
        }
        else if (!strcmp(op, MSG_INCORRECT)) {
            printf("wrong\n");
        }
        else if (!strcmp(op, MSG_END)) {
            printf("end\n");
        }
        else if (!strcmp(op, MSG_QUIT)) {
            printf("someone has quit\n");
        }
    }
}


void read_console(int out_f, int index) {
    while (1) {
        // wait for the user to answer
        int answer;
        char buf[MAX_BUF];
        char input[MAX_BUF];

        read(0, input, MAX_BUF);

        if (!strcmp(input, "quit\n")) {
            sprintf(buf, "%s%s%d", MSG_QUIT, DELIM, index);
            write(out_f, buf, MAX_BUF);   
            exit(0);
        }
        else {
            errno = 0;
            // convert the number
            answer = strtol(input, NULL, 10);

            if (errno != 0 && answer == 0) {
                // conversion error
                // TODO what about mutex?
                printf("Insert a number please\n");
            }
            else {
                // send the user answer
                sprintf(buf, "%s%s%d%s%d", MSG_ANSWER, DELIM, index, DELIM, answer);
                write(out_f, buf, MAX_BUF);
            }
        }
    }
}


int main() {
    char client_path[20] = "/tmp/";
    char buf[MAX_BUF];
    char name[NAME_LEN];

    // generate fifo name using timestamp
    // FIXME: timestamp has seconds precision
    int ts = (unsigned)time(NULL);    
    sprintf(client_path, "%s%u", client_path, ts);
    // printf("%s\n", client_path);

    // get user nickname
    printf("Enter your nickname: ");
    scanf("%10s", name);
    
    // create the FIFO 
    mkfifo(client_path, 0666);

    // open the output FIFO with write permissions and wait for the server to exist
    int out_f = open(SERVER_PATH, O_WRONLY);

    join_game(out_f, name, client_path);

    // open the input FIFO with read permissions (blocking)
    int in_f = open(client_path, O_RDONLY);
    // read the answer to the join request
    read(in_f, buf, MAX_BUF);


    // TODO use strtok to undertan the response
    int index;
    char * op = strtok(buf, DELIM);
    if (!strcmp(buf, MSG_JOIN)) {
        printf("The server didn't accepted you :(\n");
    }
    else {
        // get the index of this client on the server
        index = atoi(strtok(NULL, DELIM));
        
        // create thread for listening to the server
        pthread_t thread_id;

        // instance thread_args struct
        thread_args args;
        args.in_f = in_f;

        // create the thread
        pthread_create(&thread_id, NULL, (void*)read_fifo, (void*)&args);

        // keep sending user answer
        read_console(out_f, index);
    }


    close(in_f);
    unlink(client_path);

    return 0;

    // TODO close & unlink fifo(s)

}
