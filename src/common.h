/*
	common.h
*/


#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>

#define MAX_BUF 1024
#define NAME_LEN 11
#define SERVER_PATH "/tmp/hairy-happiness"
#define MAX_PLAYERS 10
#define MIN_PLAYERS 0
#define MAX_POINTS 100
#define MIN_POINTS 10
#define Q_FORMAT "%d + %d ="
#define DELIM ";"

// message const for communication
#define MSG_QUESTION "q"
#define MSG_ACCEPTED "accepted"
#define MSG_NOTACCEPTED "nope"
#define MSG_JOIN "join"
#define MSG_ANSWER "answer"
#define MSG_INCORRECT "no"
#define MSG_CORRECT "yes"
#define MSG_SLOW "slow"
#define MSG_END "end"
#define MSG_QUIT "quit"
#define MSG_REFH "refresh"
#define MSG_FERRARELLE "server?"
#define MSG_ROCCHETTA "alive"

// message const for communication
#define ERR_NOTACCSF -1
#define ERR_NOTACCAE -2
#define ERR_SERVERDEATH 43213

// define term colors
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

#define H_ENDGAME 13
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



/*
 return a formatted string
 */
char * toString ( const char * format, ... );
