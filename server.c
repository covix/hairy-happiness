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

#define ACTIVE_PLAYERS(x)  (sizeof(x) / sizeof(player))

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
#define MSG_QUIT "quit"
#define PL_WRONG 0
#define PL_RIGHT 1
#define PL_WIN 2



/*
    struct player used to define user's properties
*/
typedef struct player {
	int fifo;
	int point;
    int active;
	char *name;
} player;

// array of the players
player *players;
// max players acceptable
int num_players;



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
void send_question_all(char * q) {
    for (int i = 0; i < num_players; i++) {
        if (players[i].active) {
            send_question(q, players[i].fifo);
        }
    }
}


/* 
    send end information to all the client
*/
void send_end_all() {

    char * name = "";
    int point = -1;
    for (int i = 0; i < num_players; i++) {
        if (players[i].active && players[i].point > point) {
            name = players[i].name;
            point = players[i].point;
        }
    }

    for (int i = 0; i < num_players; i++) {
        if (players[i].active) {
            char buf[MAX_BUF];
            sprintf(buf, "%s%s%s", MSG_END, DELIM, name);
            write(players[i].fifo, buf, MAX_BUF);
        }
    }
}


/* 
    notify clients that one user left
*/
void send_quit_all(player pl) {
    for (int i = 0; i < num_players; i++) {
        if (players[i].active) {
            char buf[MAX_BUF];
            sprintf(buf, "%s%s%s", MSG_QUIT, DELIM, pl.name);
            write(players[i].fifo, buf, MAX_BUF);
        }
    }
}


/*
    find the first empty component in the array of the players
*/
int find_empty() {
    for (int i = 0; i < num_players; ++i)
    {
        player pl = players[i];
        if (!pl.active) {
            return i;
        }
    }
    return -1;
}



/*
 is the answer right?
 */
int answer(char buf[MAX_BUF], char * strtok_ctx, int res, int target_point, int * i, char * q) {
    // obtain player
    *i = atoi(strtok_r(NULL, DELIM, &strtok_ctx));
    printf("index %d\n", *i);
    player *pl = &players[*i];
    
    // get the answer
    int answer = atoi(strtok_r(NULL, DELIM, &strtok_ctx));
    
    if (answer == res) {
        // answer is correct
        (*pl).point++;
        
        if ((*pl).point == target_point) {
            // player is the winner
            return PL_WIN;
        }
        else {
            char msg[MAX_BUF];
            sprintf(msg, "%s%s%d", MSG_CORRECT, DELIM, (*pl).point);
            write((*pl).fifo, msg, MAX_BUF);
            return PL_RIGHT;
        }
    }
    else {
        // answer is not correct
        (*pl).point--;
        char msg[MAX_BUF];
        sprintf(msg, "%s%s%d", MSG_INCORRECT, DELIM, (*pl).point);
        write((*pl).fifo, msg, MAX_BUF);
        send_question(q, players[*i].fifo);
        return PL_WRONG;
    }
}

/*
    manage join request by clients
*/
void join(char buf[MAX_BUF], char * response, char * question)
{
    int present_players = ACTIVE_PLAYERS(players);
    player pl;
    
    
    // open output FIFO to the client
    char * fifo_path = strtok_r(NULL, DELIM, &response);
    pl.fifo = open(fifo_path, O_WRONLY);
    
    // get the nickname of the new player
    pl.name = strdup(strtok_r(NULL, DELIM, &response));
    
    
    if (present_players < num_players)
    {
        // the player is accepted 

        // calculate the point for the new client
        pl.point = num_players - present_players - 1;
        pl.active = 1;

        // add the client to the player list
        int index = find_empty();

        // empty should never be -1 at this point
        players[index] = pl;
        present_players++;
        
        // print info to server
        printf("Player join: '%s'(%d)\n", pl.name, index);

        // inform the client it has been accepted
        char msg[MAX_BUF];
        sprintf(msg, "%s%s%d", MSG_ACCEPTED, DELIM, index);
        write(pl.fifo, msg, MAX_BUF);

        // send the question to the client
        send_question(question, pl.fifo);
        
        //TODO send to all join new player

    }
    else
    {
        // request is rejected
        
        // print info to server
        printf("Player rejected (server full): '%s'\n", pl.name);
        
        // send info to client rejected
        char msg[MAX_BUF];
        sprintf(msg, MSG_NOTACCEPTED);
        write(pl.fifo, msg, MAX_BUF);
        close(pl.fifo);
    }
}





/*
    parse the max palyer number and the winning point from the parameters passed to the executable
*/
int parse_args(int argc, char *argv[], int *max_players, int *points_to_win)
{
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
                    *max_players = tmp;
                }
                else {
                    // inform the user and set to var default value
                    printf("--max argument must be between %d and %d, using %d\n", MIN_PLAYERS, MAX_PLAYERS, MAX_PLAYERS);
                    *max_players = MAX_PLAYERS;
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


int main(int argc, char *argv[])
{
    int points_to_win = 13;
    num_players = 10;
    //if (!parse_args(argc, argv, &max_players, &points_to_win)) {
        // if not all parameters are passed...
    //    printf("Usage: ./sever --max <maxplayers> --win <winningpoint\n");
        // ...close
    //    return -1;
    //}


    // TODO check for other servers running
    // add un MSG_FERRARELLE

    srand((unsigned int)time(NULL));
    
    // array of the players
    players = calloc(num_players, sizeof(*players));
    
    
    // buffer for incoming messagges
    char buf[MAX_BUF];
    // question message
    char question[strlen(Q_FORMAT) + 1];
    // result of the question
    int res = new_question(question);

    
	// create the FIFO 
    mkfifo(SERVER_PATH, 0666);
    // open the input FIFO with read and write permissions to avoid blocks
    int serverFIFO = open(SERVER_PATH, O_RDWR);

    while (1)
    {
        // read incoming messages from the FIFO
    	size_t len = read(serverFIFO, buf, MAX_BUF);
        buf[len] = '\0';
        if(strlen(buf) == 0)
            continue;
        
        printf("%s", buf);
        
        // get the operation requested by client
        char * strtok_ctx;
        char * op = strtok_r(buf, DELIM, &strtok_ctx);

        // what operation is it?
        if (!strcmp(op, MSG_JOIN))
        {
            // a new client requested to join

            // TODO check if buf is edited in debian
            join(buf, strtok_ctx, question);
        }
        else if (!strcmp(op, MSG_ANSWER))
        {
            // received answer
            int i;
            int info = answer(buf, strtok_ctx, res, points_to_win, &i, question);
            switch (info)
            {
                case PL_RIGHT:
                    // answer is right
                    res = new_question(question);
                    // send the question to everybody
                    send_question_all(question);
                    break;
                case PL_WIN:
                    // send the classification (or the winner)
                    send_end_all();
                    break;
                case PL_WRONG:
                    // answer is wrong (ah ah), maybe he didn't understand it right
                    break;
            }
        }
        else if (!strcmp(op, MSG_QUIT))
        {
            //player exit
            
            int i = atoi(strtok_r(NULL, DELIM, &strtok_ctx));
            // print info to server
            printf("Player exit: '%s'\n", players[i].name);
            players[i].active = 0;
            
            send_quit_all(players[i]);
        }
        
        
    }
    close(serverFIFO);    
    /* remove the FIFO */
    unlink(SERVER_PATH);


    return 0;
}
