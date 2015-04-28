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


/*
    send join request to server
*/
void join_game(int out_f, char * name, char * client_path) {
    char buf[MAX_BUF];
    sprintf(buf, "join %s %s", client_path, name);
    write(out_f, buf, MAX_BUF);
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

    // open the input FIFO with read and write permissions to avoid blocks
    int in_f = open(client_path, O_RDONLY);
    read(in_f, buf, MAX_BUF);
    printf("Accepted? %s\n", buf);
    close(out_f);
    // close(in_f);
    unlink(client_path);

    return 0;

    // TODO close & unlink fifo(s)

}
