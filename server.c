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
#include <getopt.h>


#define MAX_BUF 1024
#define NAME_LEN 10
#define SERVER_PATH "/tmp/hairy-happiness"
#define DELIM " "
#define MAX_PLAYERS 10
#define MIN_PLAYERS 0
#define MAX_POINTS 100
#define MIN_POINTS 10


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
    return -1;
}


void join(char buf[MAX_BUF], char * strtok_ctx, player* players,  int*active_players, int n_players) {

    player pl;
    // open output FIFO to the client
    char * fifo_path = strtok_r(NULL, DELIM, &strtok_ctx);
    pl.fifo = open(fifo_path, O_WRONLY);

    if (*active_players < n_players) { 
        // the player is accepted 

        // get the nickname of the new player
        pl.name = strtok_r(NULL, DELIM, &strtok_ctx);

        // calculate the point for the new client
        pl.pt = n_players - *active_players - 1;

        char msg[MAX_BUF];
        sprintf(msg, "accepted %d", *active_players);
        write(pl.fifo, msg, MAX_BUF);

        // TODO send the question

        // add the client to the player list
        players[*active_players++] = pl;
    }
    else {
        // request is rejected
        char msg[MAX_BUF];
        sprintf(msg, "nope");
        write(pl.fifo, msg, MAX_BUF);
    }

    printf("%d %s\n", pl.fifo, pl.name);
}


int parse_args(int argc, char *argv[], int *n_players, int *points_to_win) {
    int option_index = 0;
    int c;
    int n_flag = 0;
    int p_flag = 0;

    static struct option long_options[] = {
            {"max",  required_argument, 0,  'm' },
            {"win",  required_argument, 0,  'w' }
    };

    while ((c = getopt_long(argc, argv,"m:w:", 
                   long_options, &option_index )) != -1) {
        int tmp;
        switch (c) {
            case 'm' :
                n_flag = 1;
                tmp = atoi(optarg);
                if (tmp >= MIN_PLAYERS && tmp <= MAX_PLAYERS) {
                    *n_players = tmp;
                }
                else {
                    printf("--max argument must be between %d and %d, using %d\n", MIN_PLAYERS, MAX_PLAYERS, MAX_PLAYERS);
                    *n_players = MAX_PLAYERS;
                }
            break;
            case 'w' :
                p_flag = 1;
                tmp = atoi(optarg);
                if (tmp >= MIN_POINTS && tmp <= MAX_POINTS) {
                    *points_to_win = tmp;
                }
                else {
                    printf("--win argument must be between %d and %d, using %d\n", MIN_POINTS, MAX_POINTS, MAX_POINTS);
                    *points_to_win = MAX_POINTS;
                }
                break;
            }
    }

    return n_flag && p_flag;
}


int main(int argc, char *argv[]) {
    int points_to_win, n_players;
    if (!parse_args(argc, argv, &n_players, &points_to_win)) {
        printf("Usage: ./sever --max <maxplayers> --win <winningpoint\n");
    }

    srand(time(NULL));

    char buf[MAX_BUF];
    player players[MAX_PLAYERS];
    int active_players = 0;
    char question[8];
    int res = new_question(question);

    // TODO check for other servers running
    // TODO parse arguments

	/* create the FIFO */
    mkfifo(SERVER_PATH, 0666);

    // open the input FIFO with read and write permissions to avoid blocks
    int in_f = open(SERVER_PATH, O_RDWR);

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
            join(buf, strtok_ctx, players, &active_players, n_players);
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