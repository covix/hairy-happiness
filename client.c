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
#define DELIM ";"
#define MSG_QUESTION "q"
#define MSG_ACCEPTED "accepted"
#define MSG_NOTACCEPTED "nope"
#define MSG_JOIN "join"
#define MSG_ANSWER "answer"
#define MSG_INCORRECT "no"
#define MSG_CORRECT "yes"
#define MSG_END "end"
#define MSG_QUIT "quit"
#define MSG_REFH "refresh"


#define ERR_NOTACCSF -1
#define ERR_NOTACCAE -2

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

#define H_POINT 3
#define H_NAME 3
#define H_TLQUESTION 8
#define H_INFOQUEST 12
#define H_TLNEWS 6
#define H_QUESTION 10
#define H_NEWS 7
#define H_TLPLAYERLIST 9
#define H_SUBTLPLAYLIST 10
#define H_PLAYERLIST 11
#define H_ANSWER 11

#define ERR_SERVERDEATH 43213


pthread_mutex_t lock;

char * name;
int point;
int points_to_win;


char * toString ( const char * format, ... )
{
    char buffer[MAX_BUF];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);
    return strdup(buffer);
}

void printToCoordinates(int x, int y, char* color, const char * format, ...)
{
    char buffer[MAX_BUF];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);
    
    pthread_mutex_lock(&lock);
    
    strcpy(buffer, toString("%s\033[%d;%dH%s%s",color, y, x, strdup(buffer), RESET));
    write(2, buffer, strlen(buffer));
    
    strcpy(buffer, toString("\033[%d;19H",H_ANSWER));
    write(2, buffer, strlen(buffer));
    
    pthread_mutex_unlock(&lock);
}

char* verifyMe(char *data)
{
    if(!strcmp(data, name))
        strcpy(data, "you");
    
    return data;
}



void printPlayerList(char *data)
{
    for (int i=41;i<80;i++)
        for(int j=H_PLAYERLIST;j<20;j++)
            printToCoordinates(i, j, RESET, " ");
    
    char * list;
    char * str = strtok_r(data, "|", &list);
    if(str == NULL)
        return;
    
    int h = H_PLAYERLIST;
    do
    {
        char*namePl = verifyMe(strtok(str, ":"));
        char*pointPl = strtok(NULL, ":");
        printToCoordinates(55-(int)strlen(namePl)/2, h, RESET, namePl);
        printToCoordinates(65-(int)strlen(pointPl)/2, h, RESET, pointPl);
        
        h++;
    } while ((str = strtok_r(list, "|", &list)) != NULL);
}

void printNews(char * buf)
{
    for(int i=41;i<80;i++) printToCoordinates(i, H_NEWS, RESET, " ");
    printToCoordinates(60-(int)strlen(buf)/2, H_NEWS, RESET, buf);
}

void printNumb()
{
    for(int i=41;i<80;i++) printToCoordinates(i, H_POINT, RESET," ");
    printToCoordinates(60-((int)strlen(toString("%d",point))/2), H_POINT, RESET,toString("%d",point));
}

void clearAnswer()
{
    for(int i=0;i<40;i++) printToCoordinates(i, H_ANSWER, RESET," ");
}

void printQuestion(char *buf)
{
    for(int i=0;i<40;i++) printToCoordinates(i, H_QUESTION, RESET," ");
    printToCoordinates(20-(int)strlen(buf)/2, H_QUESTION, RESET,buf);
    clearAnswer();
}
void clearInfoQuestion()
{
    for(int i=0;i<40;i++) printToCoordinates(i, H_INFOQUEST,RESET, " ");
}

void printInfoQuestion(char *buf, int color)
{
    clearInfoQuestion();
    printToCoordinates(20-(int)strlen(buf)/2, H_INFOQUEST, (color == 0)?KGRN:KRED,buf);
}

void clearAll()
{
    write(1,"\E[H\E[2J",7);
}

void printField()
{
    clearAll();
    
    printToCoordinates(40-(strlen("THE BEST GAME OF EVA")/2), 1, KBLU, "THE BEST GAME OF EVA");
    
    printToCoordinates(20-(strlen("Your name")/2), H_NAME-1, KGRN, "Your name");
    printToCoordinates(20-((int)strlen(name)/2), H_NAME, RESET, name);
    
    printToCoordinates(60-((int)strlen(toString("Point (to win: %d)",points_to_win))/2), H_POINT-1, KGRN, "Point (to win: %d)",points_to_win);
    printNumb();
    
    printToCoordinates(20-(strlen("Question")/2), H_TLQUESTION, KYEL, "Question");
    printToCoordinates(60-(strlen("News")/2), H_TLNEWS, KYEL, "News");
    printToCoordinates(60-(strlen("Players list")/2), H_TLPLAYERLIST, KYEL, "Players list");
    printToCoordinates(55-strlen("Name")/2, H_SUBTLPLAYLIST, KRED, "Name");
    printToCoordinates(65-strlen("Point")/2, H_SUBTLPLAYLIST, KRED, "Point");
    
    
    for(int i=0;i<19;i++)
        printToCoordinates(40, 5+i, KBLU,"|");
    
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
        size_t len = read(recvFIFO, buf, MAX_BUF);
        buf[len] = '\0';
        
        
        // extract the operation
        char * data;
        char * op = strtok_r(buf, DELIM, &data);
        if (!strcmp(op, MSG_QUESTION))
        {
            printQuestion(strtok_r(NULL, DELIM, &data));
            printPlayerList(strtok_r(NULL, DELIM, &data));
        }
        else if (!strcmp(op, MSG_CORRECT))
        {
            point = atoi(strtok_r(NULL, DELIM, &data));
            printNumb();
            printInfoQuestion("Your answer is correct", 0);
        }
        else if (!strcmp(op, MSG_INCORRECT))
        {
            point = atoi(strtok_r(NULL, DELIM, &data));
            printNumb();
            clearAnswer();
            printInfoQuestion("Your answer is not correct", 1);
        }
        else if (!strcmp(op, MSG_JOIN))
        {
            char *nameJoined = verifyMe(strtok_r(NULL, DELIM, &data));
            int pointJoined = atoi(strtok_r(NULL, DELIM, &data));
            
            printNews(toString("%s join with %d points", nameJoined, pointJoined));
            printPlayerList(strtok_r(NULL, DELIM, &data));
        }
        else if (!strcmp(op, MSG_END))
        {
            char *nameWinner = verifyMe(strtok_r(NULL, DELIM, &data));
            int pointWinner = atoi(strtok_r(NULL, DELIM, &data));
            
            printNews(toString("%s win! with %d points", nameWinner, pointWinner));
            printPlayerList(strtok_r(NULL, DELIM, &data));
        }
        else if (!strcmp(op, MSG_QUIT))
        {
            printNews(toString("%s left", verifyMe(strtok_r(NULL, DELIM, &data))));
            printPlayerList(strtok_r(NULL, DELIM, &data));
        }
        else if (!strcmp(op, MSG_REFH))
        {
            printPlayerList(strtok_r(NULL, DELIM, &data));
        }
        //TODO server close
    }
}

ssize_t writeLella(int sendFIFO, char * text)
{
    ssize_t erro = write(sendFIFO, text, MAX_BUF);
    if(erro == -1)
    {
        // print info to server
        clearAll();
        printf(KGRN"Server death\n"RESET);
        erro = ERR_SERVERDEATH;
    }
    
    return erro;
}

void read_console(int sendFIFO, int index)
{
    while (1)
    {
        // wait for the user to answer
        char input[MAX_BUF];
        size_t len = read(0, input, MAX_BUF);
        input[len] = '\0';
        
        clearInfoQuestion();
        
        
        if (!strcmp(input, "quit\n"))
        {
            write(sendFIFO, toString("%s%s%d", MSG_QUIT, DELIM, index), MAX_BUF);
            clearAll();
            printf(KGRN"You exit\n"RESET);
            break;
        }
        else
        {
            errno = 0;
            // convert the number
            long answer = strtol(input, NULL, 10);

            if (errno != 0 && answer == 0)// conversion error
            {
                printInfoQuestion("Insert a number please", 1);
                clearAnswer();
            }
            else // send the user answer
            {
                if(writeLella(sendFIFO, toString("%s%s%d%s%ld", MSG_ANSWER, DELIM, index, DELIM, answer)) == ERR_SERVERDEATH)
                    break;
            }
        }
    }
}



int main()
{
    // get user nickname
    name = calloc(NAME_LEN, sizeof(*name));
    printf("Enter your nickname: ");
    scanf("%10s", name);
    
    signal(SIGPIPE, SIG_IGN);
    pthread_mutex_init(&lock, NULL);
    
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
        point = atoi(strtok(NULL, DELIM));
        points_to_win = atoi(strtok(NULL, DELIM));
        
        
        printField();
        
        
        // create thread for listening to the server
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, (void*)read_fifo, (void*)&recvFIFO);

        // keep sending user answer
        read_console(sendFIFO, index);
    }

    
    unlink(client_path);
    pthread_mutex_destroy(&lock);


    return 0;
}
