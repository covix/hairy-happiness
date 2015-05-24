/*
	project.c
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "server.h"
#include "client.h"


#define MAX_BUF 1024


// used for changing the output mode
int TEST = 0;


int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("client or server?\n");
		return -1;
	}

	int i = 1;
	if (!strcmp(argv[i], "test")) {
		// set 'test' to true and read the behaviour (client or server) in the next par
		TEST = 1;
		i++;
	}

	if (!strcmp(argv[i], "client")) {
		// launch client
		main_client();
	}
	else if (!strcmp(argv[i], "server")) {
		// launch server
		main_server(argc, argv);

	}
	else {
		printf("Usage: ./proj <client | server> <opt>\n");
		return -1;
	}
}
