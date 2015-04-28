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
void interact_with_game(int in_f, int out_f, int index) {
    while(1) {
        // read message
        char buf[MAX_BUF];
        read(in_f, buf, MAX_BUF);

        // extract the op
        char * buf_dup = strdup(buf);
        char * op = strtok(buf, DELIM);

        if (!strcmp(op, MSG_QUESTION)) {
            // obtain only the question
            buf_dup += 2;
            write(1, buf_dup, strlen(buf_dup));
            
            // wait for the user to answer
            char answer[MAX_BUF];
            char answer_buf[MAX_BUF];
            scanf("%s", answer);
            sprintf(answer_buf, "%s%s%d%s%s", MSG_ANSWER, DELIM, index, DELIM, answer);
            printf("%s\n", answer_buf);

            // send answer
            write(out_f, answer_buf, MAX_BUF);
        }
        else if (!strcmp(op, MSG_CORRECT)) {
        }
        else if (!strcmp(op, MSG_INCORRECT)) {
            printf("sbagliato\n");
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

        interact_with_game(in_f, out_f, index);
    }


    close(in_f);
    unlink(client_path);

    return 0;

    // TODO close & unlink fifo(s)

}
