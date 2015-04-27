/* 
    server.h
*/

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF 1024
#define NAME_LEN 10
#define SERVER_PATH "/tmp/hairy-happiness"


typedef struct {
	int fifo;
	int pt;
	char name[NAME_LEN];
} player;


int main()
{
    int fd;                          	
    char myfifo[] = "/tmp/myfifo";
    char buf[MAX_BUF];
    player * players;

	/* create the FIFO */
    mkfifo(SERVER_PATH, 0666);

    /* open, read, and display the message from the FIFO */
    fd = open(SERVER_PATH, O_RDWR);

    while(1) {
    	read(fd, buf, MAX_BUF);
    	printf("Received: %s\n", buf);	
    }
    close(fd);    
    /* remove the FIFO */
    unlink(myfifo);


    return 0;
}
