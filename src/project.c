#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (!strcmp(argv[1], "client")) {
		// launch client
		printf("client\n");
	}
	else if (!strcmp(argv[1], "server")) {
		// launch server
		printf("server\n");
	}
	else {
		printf("Usage: ./proj <client | server> <opt>\n");
		return -1;
	}
}