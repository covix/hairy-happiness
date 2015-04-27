#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define MAX_BUF 1024
#define NAME_LEN 10

int main()
{
    // generate fifo name using timestamp
    // TODO: timestamp has seconds precision
    int fd;
    int ts = (unsigned)time(NULL);
    char myfifo[] = "/tmp/";
    sprintf(myfifo, "%s%d", myfifo, ts);
    
    char name[NAME_LEN];

    printf("Enter your nickname: ");
    scanf("%10s", name);

    char buf[MAX_BUF];

    printf("%s\n", myfifo);

    return 0;


    /* write "Hi" to the FIFO */
    fd = open(myfifo, O_WRONLY);
    scanf ("%79s", buf); 
    printf("sending");

    while(1) {
        // scanf("%1s", bla);
        write(fd, buf, MAX_BUF);
    }
    close(fd);

    /* remove the FIFO */
    // unlink(myfifo);

    return 0;
}
