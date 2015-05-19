/*
	project.c
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"
#include "server.h"
#include "client.h"


#define MAX_BUF 1024

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("client or server?\n");
		return -1;
	}

	if (!strcmp(argv[1], "client")) {
		// launch client
		main_client();
	}
	else if (!strcmp(argv[1], "server")) {
		// launch server
		main_server(argc, argv);

	}
	else {
		printf("Usage: ./proj <client | server> <opt>\n");
		return -1;
	}
}