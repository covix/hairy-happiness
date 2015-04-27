/*
    client.h
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

// FIFO write operation are atomic only using at most 1024 bytes
#define MAX_BUF 1024
#define NAME_LEN 10
#define SERVER_PATH "/tmp/hairy-happiness"


void join_game(int out_f, char * name, int in_f) {
    char buf[MAX_BUF];
    sprintf(buf, "%s;%d", name, in_f);
    write(out_f, buf, MAX_BUF);
}

int main()
{
    char client_path[] = "/tmp/";
    char buf[MAX_BUF];
    char name[NAME_LEN];

    // generate fifo name using timestamp
    // TODO: timestamp has seconds precision
    int ts = (unsigned)time(NULL);    
    sprintf(client_path, "%s%u", client_path, ts);
    // printf("%s\n", client_path);

    // get user nickname
    printf("Enter your nickname: ");
    scanf("%10s", name);
    
    // create the FIFO 
    mkfifo(client_path, 0666);

    // open the output FIFO with write permissions
    int out_f = open(SERVER_PATH, O_WRONLY);

    join_game(out_f, name, -1);

    // open the input FIFO with read permissions
    // int in_f = open(client_path, O_RDONLY);

    close(out_f);
    // close(in_f);
    unlink(client_path);

    return 0;


    // TODO close & unlink fifo(s)

}
