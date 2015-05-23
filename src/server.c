/* 
    server.c
*/



#include "common.h"


ssize_t writeIfActive(int index, char * text);


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

// point to win
int pointsToWinServer;

// question message
char* question;

// result of the question
int res;


/*
    generate new question and result
*/
int createQuestion(char * q)
{
    int n1 = rand() % 100;
    int n2 = rand() % 100;
    sprintf(q, Q_FORMAT, n1, n2);

    return n1 + n2;
}

//ciao:2|paos:3|desni:3|

/*
    return the string representation of all the players
*/
char* encodeListPlayers()
{
    char list[MAX_BUF];
    strcpy(list, "");

    for(int i=0;i<num_players;i++)
        if(players[i].active)
            strcat(list, toString("%s:%d|", players[i].name, players[i].point));
    
    // return a copy of the string
    return strdup(list);
}


/* 
    send the question to all the client
*/
void sendQuestionToAll()
{
    for (int i = 0; i < num_players; i++)
        if (players[i].active)
            writeIfActive(i, toString("%s%s%s%s%s", MSG_QUESTION, DELIM, question, DELIM, encodeListPlayers()));
}

/* 
    send end information to all the client
*/
void sendEndGameToAll(player winner)
{
    for (int i = 0; i < num_players; i++)
        if (players[i].active)
            writeIfActive(i, toString("%s%s%s%s%d%s%s", MSG_END, DELIM, winner.name, DELIM, winner.point, DELIM, encodeListPlayers()));
}


/* 
    notify clients that one user left
*/
void sendQuitToAll(player pl)
{
    for (int i = 0; i < num_players; i++)
        if (players[i].active)
            writeIfActive(i, toString("%s%s%s%s%s", MSG_QUIT, DELIM, pl.name, DELIM, encodeListPlayers()));
}


/*
    keep the classify updated
*/
void sendRefreshScoreToAll()
{
    for (int i = 0; i < num_players; i++)
        if (players[i].active)
            writeIfActive(i, toString("%s%s%s", MSG_REFH, DELIM, encodeListPlayers()));
}


/*
 notify clients that one user join
 */
void sendJoinToAll(player pl)
{
    for (int i = 0; i < num_players; i++)
        if (players[i].active)
            writeIfActive(i, toString("%s%s%s%s%d%s%s", MSG_JOIN, DELIM, pl.name, DELIM, pl.point, DELIM, encodeListPlayers()));
}


/*
    write 'text' to the player with index 'index' and check if it is still active; if it isn't, notify all the client about his death
*/
ssize_t writeIfActive(int index, char * text)
{
    ssize_t erro = write(players[index].fifo, text, MAX_BUF);
    if(erro == -1)
    {
        // print info to server
        printf(KGRN"Player death: '%s'\n"RESET, players[index].name);
        
        // disable the user
        players[index].active = 0;

        // inform to all of a user's death
        sendQuitToAll(players[index]);
    }
    
    return erro;
}


/*
    Respect the old one! I've been the server since tests! I could be the one who forked you!
*/
void thisIsMyTerritory(char * response)
{
        player pl;
        // open output FIFO to the fake server
        pl.fifo = open(strtok_r(NULL, DELIM, &response), O_WRONLY);
        
        // write to the just opened server that I already exist   
        write(pl.fifo, toString("%s%s", MSG_ROCCHETTA, DELIM), MAX_BUF);
        close(pl.fifo);
}


/*
    is the answer right? well, if so, notify all the notifiable things to all the client
    even if it isn't
 */
void answer(char * response)
{
    // obtain player
    int index = atoi(strtok_r(NULL, DELIM, &response));
    // get the answer
    int answer = atoi(strtok_r(NULL, DELIM, &response));
    
    
    if (answer == res) // answer is correct
    {
        players[index].point++;
        
        if (players[index].point == pointsToWinServer) // player is the winner
        {
            printf(KBLU"The winner is %s with %d point\n"RESET, players[index].name, players[index].point);
            
            sendEndGameToAll(players[index]);
            
            printf(KGRN"End Server\n"RESET);
            exit(1);
        }
        else // answer is right
        {
            printf(KBLU"Correct answer by %s (point:%d)\n"RESET, players[index].name, players[index].point);
            
            writeIfActive(index, toString("%s%s%d", MSG_CORRECT, DELIM, players[index].point));
            for (int i = 0; i < num_players; i++)
                if (players[i].active && i != index)
                    writeIfActive(i, toString("%s", MSG_SLOW));
            
            // create and send the question to everybody
            res = createQuestion(question);
            printf(KBLU"Generate other question: %s (%d)\n"RESET, question, res);
            sendQuestionToAll();
        }
    }
    else // answer is not correct
    {
        players[index].point--;
        
        
        printf(KBLU"Incorrect answer by %s (point:%d)\n" RESET, players[index].name, players[index].point);
        
        writeIfActive(index, toString("%s%s%d", MSG_INCORRECT, DELIM, players[index].point));
        sendRefreshScoreToAll();
    }
}


/*
    Check if the clients are still alive
*/
void refreshAlive()
{
    for (int i = 0; i < num_players; i++)
        if (players[i].active)
            writeIfActive(i, MSG_FERRARELLE);
}


/*
    find the first empty component in the array of the players
 */
int indexOfEmptyAndNoNameAlreadyExist(char *name)
{
    for (int i = 0; i < num_players; i++)
        if (players[i].active)
            if (!strcmp(players[i].name, name))
                return ERR_NOTACCAE;
    
    for (int i = 0; i < num_players; ++i)
        if (!players[i].active)
            return i;
    
    return ERR_NOTACCSF;
}


/*
    return the number of active players
 */
int countPlayer()
{   
    refreshAlive();
    int count = 0;
    for (int i = 0; i < num_players; ++i)
        if (players[i].active)
            count++;
    
    return count;
}


/*
    manage join request by clients
*/
void joinPlayer(char * response)
{
    player pl;
    // open output FIFO to the client
    pl.fifo = open(strtok_r(NULL, DELIM, &response), O_WRONLY);
    // get the nickname of the new player
    pl.name = strdup(strtok_r(NULL, DELIM, &response));
    // calculate the point for the new client
    pl.point = num_players - countPlayer() - 1;
    // active player
    pl.active = 1;

    refreshAlive();
    int index = indexOfEmptyAndNoNameAlreadyExist(pl.name);

    if (index > -1) // the player is accepted
    {
        // add the client to the player list
        players[index] = pl;
        
        // print info to server
        printf(KGRN "Player join: '%s'(%d) with %d point\n" RESET, pl.name, index, pl.point);

        // inform the client it has been accepted
        writeIfActive(index, toString("%s%s%d%s%d%s%d", MSG_ACCEPTED, DELIM, index, DELIM, pl.point, DELIM, pointsToWinServer));

        // send the question to the client
        writeIfActive(index, toString("%s%s%s%s%s", MSG_QUESTION, DELIM, question, DELIM, encodeListPlayers()));
        
        // send to all join new player
        sendJoinToAll(players[index]);
    }
    else // request is rejected
    {
        // print info to server
        if(index == ERR_NOTACCSF)
            printf(KGRN "Player rejected: (server full): '%s'\n" RESET, pl.name);
        else if(index == ERR_NOTACCAE)
        {
            printf(KGRN "Player rejected: (name already exist): '%s'\n" RESET, pl.name);
        }
        
        // send info to client rejected
        write(pl.fifo,toString("%s%s%d", MSG_NOTACCEPTED, DELIM, index), MAX_BUF);
        close(pl.fifo);
    }
}


/*
    a player left the game, notify everybody math is not for him
*/
void leftPlayer(char * response)
{
    int index = atoi(strtok_r(NULL, DELIM, &response));
    
    // print info to server
    printf(KGRN "Player exit: '%s'\n" RESET, players[index].name);
    
    // disable the user
    players[index].active = 0;
    close(players[index].fifo);
    
    // inform to all of a user's left
    sendQuitToAll(players[index]);
}


/*
    On startup check if there's another server running
*/
int otherServerIsUp()
{
    char server_tmp_path[30];
    // generate fifo name using timestamp
    sprintf(server_tmp_path, "/tmp/%u", (unsigned)time(NULL));
    // create the FIFOs
    mkfifo(server_tmp_path, 0666);    // open the input FIFO with read permissions (blocking)
    int recvFIFO = open(server_tmp_path, O_RDWR);

    // open the output FIFO with write permissions and wait for the server to exist
    int sendFIFO = open(SERVER_PATH, O_RDWR);

    // look for server
    write(sendFIFO, toString("%s%s%s", MSG_FERRARELLE, DELIM, server_tmp_path), MAX_BUF);

    struct pollfd fds[1];
    int timeout_msecs = 2 * 1000;
    int i;

    fds[0].fd = recvFIFO;
    fds[0].events = POLLIN;

    printf("Looking for server already running...\n");
    i = poll(fds, 1, timeout_msecs);

    if (i > 0)
    {
        // read the answer of an already present server
        char buf[MAX_BUF];
        size_t len = read(recvFIFO, buf, MAX_BUF);
        buf[len] = '\0';
        printf("Another server is already running\n");
    }
    return i;
}


/*
    parse the max palyer number and the winning point from the parameters passed to the executable
*/
int parse_args(int argc, char *argv[])
{
    int option_index = 0;
    int c;
    int n_flag = 0;
    int p_flag = 0;
    
    // define the params that the server expects
    static struct option long_options[] =
    {
        {"max",  required_argument, 0,  'm' },
        {"win",  required_argument, 0,  'w' }
    };
    
    // iterate over the parameters
    while ((c = getopt_long(argc, argv,"m:w:", long_options, &option_index )) != -1)
    {
        int tmp;
        switch (c)
        {
            case 'm' : /* --max */
                // param found
                n_flag = 1;
                // convert parameter to int
                tmp = atoi(optarg);
                // check restrictions
                if (tmp >= MIN_PLAYERS && tmp <= MAX_PLAYERS) {
                    num_players = tmp;
                }
                else {
                    // inform the user and set to var default value
                    printf("--max argument must be between %d and %d, using %d\n", MIN_PLAYERS, MAX_PLAYERS, MAX_PLAYERS);
                    num_players = MAX_PLAYERS;
                }
                break;
            case 'w' : /* --win */
                p_flag = 1;
                tmp = atoi(optarg);
                // check restrictions
                if (tmp >= MIN_POINTS && tmp <= MAX_POINTS) {
                    pointsToWinServer = tmp;
                }
                else {
                    // inform the user and set to var default value
                    printf("--win argument must be between %d and %d, using %d\n", MIN_POINTS, MAX_POINTS, MAX_POINTS);
                    pointsToWinServer = MAX_POINTS;
                }
                break;
        }
    }
    
    // check whether all params where passed
    return n_flag && p_flag;
}


/* 
    Entry point of the program
*/
int main_server(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    srand((unsigned int)time(NULL));


    if (!parse_args(argc, argv)) {
        // if not all parameters are passed...
        printf("Usage: ./sever --max <maxplayers> --win <winningpoint>\n");
        // ...close
        return -1;
    }
    
    // used for debug
    // pointsToWinServer = 13;
    // num_players = 10;
    

    // check for other servers running
    if(otherServerIsUp() != 0)
        return -1;
    
    // array of the players
    players = calloc(num_players, sizeof(*players));
    // question message
    question = calloc(strlen(Q_FORMAT) + 1, sizeof(*question));
    // result of the question
    res = createQuestion(question);

    
	// create the FIFO 
    mkfifo(SERVER_PATH, 0666);
    // open the input FIFO with read and write permissions to avoid blocks
    int serverFIFO;
    if((serverFIFO = open(SERVER_PATH, O_RDWR)) != -1)
        printf("Server started...\n"KBLU "First question generated: %s (%d)\n" RESET, question, res);
    
    
    while (serverFIFO != -1)
    {
        // buffer for incoming messagges
        char buf[MAX_BUF];
        size_t len;
        
        // read incoming messages from the FIFO
    	if((len = read(serverFIFO, buf, MAX_BUF)) <= 0) continue;
        buf[len] = '\0';
        
        // get the operation requested by client
        char * data;
        char * operation = strtok_r(buf, DELIM, &data);

        // what operation is it?
        if (!strcmp(operation, MSG_JOIN)) 
        {
            // a new client requested to join
            joinPlayer(data);
        }
        else if (!strcmp(operation, MSG_QUIT)) 
        {
            // a client left
            leftPlayer(data);
        }
        else if (!strcmp(operation, MSG_ANSWER))
        {
            // received answer
            answer(data);
        }
        else if (!strcmp(operation, MSG_FERRARELLE))
        {
            // a new server is looking for older one
             thisIsMyTerritory(data);
        }
        else {
            printf(KRED"Received message not expected\n"RESET);
        }
        
    }
    
    close(serverFIFO);    
    /* remove the FIFO */
    unlink(SERVER_PATH);

    return 0;
}
