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
#define NAME_LEN 11
#define SERVER_PATH "/tmp/hairy-happiness"
#define MAX_PLAYERS 10
#define MIN_PLAYERS 0
#define MAX_POINTS 100
#define MIN_POINTS 10
#define Q_FORMAT "%d + %d: "
#define DELIM " "
#define MSG_QUESTION "q"
#define MSG_ACCEPTED "accepted"
#define MSG_NOTACCEPTED "nope"
#define MSG_JOIN "join"
#define MSG_ANSWER "answer"
#define MSG_INCORRECT "no"
#define MSG_CORRECT "yes"
#define MSG_END "end"
#define PL_WIN -1
#define PL_RIGHT -2


/*
    struct player used to define user's properties
*/
typedef struct player {
	int fifo;
	int pt;
	char *name;
} player;


/*
    generate new question and result
*/
int new_question(char * q) {
    int n1 = rand() % 100;
    int n2 = rand() % 100;
    sprintf(q, Q_FORMAT, n1, n2);

    return n1 + n2;
}


/* 
    write the question to the passed fifo
*/
void send_question(char * q, int fifo) {
    char buf[MAX_BUF];
    sprintf(buf, "%s%s%s", MSG_QUESTION, DELIM, q);
    write(fifo, buf, MAX_BUF);
}


/* 
    send the question to all the client
*/
void send_question_all(char * q, player * players, int n_players, int * players_active) {
    for (int i = 0; i < n_players; i++) {
        if (players_active[i]) {
            send_question(q, players[i].fifo);
        }
    }
}


/* 
    send end information to all the client
*/
void send_end_all(player * players, int n_players, int * players_active) {

    char * name;
    int pt = -1;
    for (int i = 0; i < n_players; i++) {
        if (players_active[i] && players[i].pt > pt) {
            name = players[i].name;
            pt = players[i].pt;
        }
    }

    for (int i = 0; i < n_players; i++) {
        if (players_active[i]) {
            char buf[MAX_BUF];
            sprintf(buf, "%s%s%s", MSG_END, DELIM, name);
            write(players[i].fifo, buf, MAX_BUF);
        }
    }
}


/*
    find the first empty component in the array of the players
*/
int find_empty(int * players_active, int n_players) {
    for (int i = 0; i < n_players; ++i) {
        if (!players_active[i]) {
            return i;
        }
    }
    return -1;
}


/*
    manage join request by clients
*/
void join(char buf[MAX_BUF], char * strtok_ctx, player players[],  int*present_players, int n_players, int * players_active, char * q) {

    player pl;
    // open output FIFO to the client
    char * fifo_path = strtok_r(NULL, DELIM, &strtok_ctx);
    pl.fifo = open(fifo_path, O_WRONLY);

    if (*present_players < n_players) { 
        // the player is accepted 

        // get the nickname of the new player
        pl.name = strtok_r(NULL, DELIM, &strtok_ctx);

        // calculate the point for the new client
        pl.pt = n_players - *present_players - 1;
        printf("pt: %d\n", pl.pt);

        // add the client to the player list
        int empty = find_empty(players_active, n_players);

        // empty should never be -1 at this point
        players[empty] = pl;
        players[empty].name = strdup(pl.name);

        players_active[empty] = 1;
        (*present_players)++;

        // inform the client it has been accepted
        char msg[MAX_BUF];
        sprintf(msg, "%s%s%d", MSG_ACCEPTED, DELIM, empty);
        write(pl.fifo, msg, MAX_BUF);

        // send the question to the client
        send_question(q, pl.fifo);

    }
    else {
        // request is rejected
        char msg[MAX_BUF];
        sprintf(msg, MSG_NOTACCEPTED);
        write(pl.fifo, msg, MAX_BUF);
        close(pl.fifo);
    }

    printf("%d %s\n", pl.fifo, pl.name);
}


/*
    is the answer right?
*/
int answer(char buf[MAX_BUF], char * strtok_ctx, player players[], int res, int target_pt) {
    // obtain player
    int i = atoi(strtok_r(NULL, DELIM, &strtok_ctx));
    printf("index %d\n", i);
    player *pl = &players[i];

    // get the answer
    int answer = atoi(strtok_r(NULL, DELIM, &strtok_ctx));

    if (answer == res) {
        // answer is correct
        (*pl).pt++;

        if ((*pl).pt == target_pt) {
            // player is the winner
            return PL_WIN;
        }
        else {
            return PL_RIGHT;
        }
    }
    else {
        // answer is not correct
        (*pl).pt--;
        char msg[MAX_BUF];
        sprintf(msg, "%s%s%d", MSG_INCORRECT, DELIM, (*pl).pt);
        write((*pl).fifo, msg, MAX_BUF);
        return i;    
    }
}


/*
    parse the max palyer number and the winning point from the parameters passed to the executable
*/
int parse_args(int argc, char *argv[], int *n_players, int *points_to_win) {
    int option_index = 0;
    int c;
    int n_flag = 0;
    int p_flag = 0;

    // define the params that the server expects
    static struct option long_options[] = {
            {"max",  required_argument, 0,  'm' },
            {"win",  required_argument, 0,  'w' }
    };

    // iterate over the parameters
    while ((c = getopt_long(argc, argv,"m:w:", 
                   long_options, &option_index )) != -1) {
        int tmp;
        switch (c) {
            case 'm' : /* --max */
                // param found
                n_flag = 1;
                // convert parameter to int
                tmp = atoi(optarg);
                // check restrictions
                if (tmp >= MIN_PLAYERS && tmp <= MAX_PLAYERS) {
                    *n_players = tmp;
                }
                else {
                    // inform the user and set to var default value
                    printf("--max argument must be between %d and %d, using %d\n", MIN_PLAYERS, MAX_PLAYERS, MAX_PLAYERS);
                    *n_players = MAX_PLAYERS;
                }
            break;
            case 'w' : /* --win */
                p_flag = 1;
                tmp = atoi(optarg);
                // check restrictions
                if (tmp >= MIN_POINTS && tmp <= MAX_POINTS) {
                    *points_to_win = tmp;
                } 
                else {
                    // inform the user and set to var default value
                    printf("--win argument must be between %d and %d, using %d\n", MIN_POINTS, MAX_POINTS, MAX_POINTS);
                    *points_to_win = MAX_POINTS;
                }
                break;
            }
    }

    // check whether all params where passed
    return n_flag && p_flag;
}


int main(int argc, char *argv[]) {
    int points_to_win, n_players;
    if (!parse_args(argc, argv, &n_players, &points_to_win)) {
        // if not all parameters are passed...
        printf("Usage: ./sever --max <maxplayers> --win <winningpoint\n");
        // ...close
        return -1;
    }


    // TODO check for other servers running


    srand(time(NULL));

    // buffer for incoming messagges
    char buf[MAX_BUF];
    // array of the players
    player players[n_players];
    // array to store if players are present/active
    // calloc set memory to "0"
    int *players_active = (int*)calloc(n_players, sizeof(int));
    // number of present player
    int present_players = 0;
    // question message
    char question[strlen(Q_FORMAT) + 1];
    // result of the question
    int res = new_question(question);

	// create the FIFO 
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
        if (!strcmp(op, MSG_JOIN)) {
            // a new client requested to join

            // TODO check if buf is edited in debian
            join(buf, strtok_ctx, players, &present_players, n_players, players_active, question);

        }
        else if (!strcmp(op, MSG_ANSWER)) {
            // received answer
            int info = answer(buf, strtok_ctx, players, res, points_to_win);
            switch (info) {
                case PL_RIGHT:
                    // answer is right
                    res = new_question(question);
                    // send the question to everybody
                    send_question_all(question, players, n_players, players_active);
                    break;
                case PL_WIN:
                    // send the classification (or the winner)
                    send_end_all(players, n_players, players_active);
                    break;
                default:
                    // answer is wrong (ah ah), maybe he didn't understand it right
                    send_question(question, players[info].fifo);
                    break;
            }
        }
    }
    close(in_f);    
    /* remove the FIFO */
    unlink(SERVER_PATH);


    return 0;
}
