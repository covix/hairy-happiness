#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF 1024
#define NAME_LEN 10


typedef struct {
	int fifo;
	int pt;
	char name[NAME_LEN];
} player;


int main()
{
    int fd;                          	
    char * myfifo = "/tmp/myfifo";
    char buf[MAX_BUF];

	/* create the FIFO (named pipe) */
    mkfifo(myfifo, 0666);

    /* open, read, and display the message from the FIFO */
    fd = open(myfifo, O_RDONLY);
    while(1) {
    	scanf("%1s", bla);
    	read(fd, buf, MAX_BUF);
    	printf("Received: %s\n", buf);	
    }
    close(fd);    
    /* remove the FIFO */
    unlink(myfifo);


    return 0;
}
