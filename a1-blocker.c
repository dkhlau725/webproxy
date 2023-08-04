// CPSC 441 Assignment 1 (Fall 2021)
// By Desmond Lau (30089787)
// Dynamic blocker program for main proxy

#define PROXY_IP "136.159.5.27"
#define BLOCKER_PORT 5678

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>	
#include <netdb.h>
#include <stdbool.h>

// simple program that sends inputted keywords to the proxy to manage.
int main(int argc, char *argv[]) {
    int sock;
	struct sockaddr_in server;
	char blockWord[100];
	
	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Could not create socket\n");
		return 1;
	}
	puts("Blocker socket created.\n");
	
	server.sin_addr.s_addr = inet_addr(PROXY_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(BLOCKER_PORT);

	//Connect to remote server
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("connect failed. Error");
		return 1;
	}	
	puts("Connected to proxy.\n");

    // have a loop to constantly look for content to block
	printf("To block a word, type <word>. (not case sensitive) \nTo unblock all words, type UNBLOCK. \nTo close the blocker and proxy, type EXIT. \n");
	while(1) {
		scanf("%s", blockWord);
		if(blockWord != NULL) {
			if(send(sock, blockWord, 100, 0) < 0) {
				puts("Send failed\n");
				return 1;
			}
			if (strstr(blockWord, "EXIT") != NULL) {
				break;
			}
		}
		else {
			continue;
		}
	}
}