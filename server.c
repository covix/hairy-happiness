/* 
    server.h
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


#define MAX_BUF 1024
#define NAME_LEN 10
#define SERVER_PATH "/tmp/hairy-happiness"
#define DELIM " "
#define MAX_PLAYERS 10


typedef struct {
	int fifo;
	int pt;
	char * name;
} player;


int new_question(char * q) {
    int n1 = rand() % 100;
    int n2 = rand() % 100;
    sprintf(q, "%d + %d", n1, n2);

    return n1 + n2;
}


int answer(char buf[MAX_BUF], char * strtok_ctx, player* players) {
    // TODO write me
}


void join(char buf[MAX_BUF], char * strtok_ctx, player* players,  int &n_players) {

    player pl;
    // open output FIFO to the client
    char * fifo_path = strtok_r(NULL, DELIM, &strtok_ctx);
    pl.fifo = open(fifo_path, O_WRONLY);

    if (n_players < MAX_PLAYERS) { 
        // the player is accepted 

        // get the nickname of the new player
        pl.name = strtok_r(NULL, DELIM, &strtok_ctx);

        // calculate the point for the new client
        pl.pt = MAX_PLAYERS - n_players - 1;

        char msg[MAX_BUF];
        sprintf(msg, "accepted %d", n_players);
        write(pl.fifo, msg, MAX_BUF);

        // TODO send the question

        // add the client to the player list
        players[n_players++] = pl;
    }
    else {
        // request is rejected
        char msg[MAX_BUF];
        sprintf(msg, "nope");
        write(pl.fifo, msg, MAX_BUF);
    }

    printf("%d %s\n", pl.fifo, pl.name);
}


int main() {
    srand(time(NULL));

    int in_f;
    char buf[MAX_BUF];
    player players[MAX_PLAYERS];
    int n_players = 0;
    char[8] question;
    int res = new_question(question);

    // TODO check for other servers running
    // TODO parse arguments

	/* create the FIFO */
    mkfifo(SERVER_PATH, 0666);

    // open the input FIFO with read and write permissions to avoid blocks
    in_f = open(SERVER_PATH, O_RDWR);

    while (1) {
        // read incoming messages from the FIFO
    	read(in_f, buf, MAX_BUF);
        printf("Received: %s\n", buf);

        // get the operation requested by client
        char * strtok_ctx;
        char * op = strtok_r(buf, DELIM, &strtok_ctx);

        // what operation is it?
        if (!strcmp(op, "join")) {
            // a new client requested to join

            // TODO check if buf is edited in debian
            join(buf, strtok_ctx, players, n_players);
        } 
        else if (!strcmp(op, "try")) {
            // received answer
            answer(buf, strtok_ctx, players);
        }
    }
    close(in_f);    
    /* remove the FIFO */
    unlink(SERVER_PATH);


    return 0;
}
