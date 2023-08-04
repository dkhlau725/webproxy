// CPSC 441 Assignment 1 (Fall 2021)
// By Desmond Lau (30089787)
// Main proxy program

#define PROXY_IP "136.159.5.27"
#define BLOCKER_PORT 5678
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <netinet/in.h>
#include <unistd.h>	
#include <netdb.h>
#include <stdbool.h>
#include <fcntl.h>
#include <ctype.h>

char *blockList[100], blockWord[100], *httpArray[50], *tempArray[50];
char *keyword;
int blockWordCount = 0, i = 0;
char* url, *host, *path, *split;

// function to extract the URL, host and path name out of the client GET request
void parseHTTP(char *client_message) {
	//split client message by new line and store in array
	split = strtok(client_message, "\n");
	while (split != NULL) {
		httpArray[i++] = split;			
		split = strtok(NULL, "\n");
	}

	// access the first line containing the URL and split by spaces and store in array
	split = strtok(httpArray[0], " ");
	i = 0;
	while (split != NULL) {
		tempArray[i++] = split;
		split = strtok(NULL, " ");
	}

	// extract the URL and then the host and path
	url = tempArray[1]+7;
	split = calloc(strlen(url)+1, sizeof(char));
	strcpy(split, url);
	host = strtok(split, "/");
	path = url + strlen(host);
					
	printf("url = %s\n", (char*)url);
	printf("host = %s\n", (char*)host);
	printf("path = %s\n", (char*)path);		
}

// function to update the list of restricted words. returns 0 if program terminating, 1 otherwise.
int updateBlocker() {
	printf("BLOCKER SENDS: %s\n", blockWord);

	// if user wants to exit, return 0 so main can close proxy properly
	if(strstr(blockWord, "EXIT") != NULL) {
		printf("Proxy Closing. Goodbye Internet\n");
		return 0;
	}
	// if user wants to block a keyword, add to list of block words (doesn't check for duplicates)
	else if(strstr(blockWord, "UNBLOCK") == NULL && blockWordCount < 100) {
		keyword = calloc(strlen(blockWord)+1, sizeof(char));
		strcpy(keyword, blockWord);
		printf("ADDING TO BLOCK LIST: %s\n", keyword);
		blockList[blockWordCount] = keyword;
		blockWordCount++;
	}
	// if user wants to clear the block list
	else {
		memset(blockList, 0, sizeof(blockList));
		blockWordCount = 0;
	}

	printf("\nCONTENTS of block list:\n");
	for (int i = 0; i < 100; i++) {
		if (blockList[i] != NULL) {
			printf("%s\n", blockList[i]);
		}
	}
	printf("\n");

	return 1;
}

// main function containing the while loop to maintain connections with client browser and web server
int main(int argc, char *argv[]){
	int proxy_server_sock, client_sock, proxy_client_sock, blocker_sock, proxy_blocker_sock;
	struct sockaddr_in server, webServer, blockerLink;
	struct hostent *hostStruct;
	char client_message[5000], serverReply[2048];
	int opt = 1;
	int recvStatus, recvStatus2;
	int userPort;
	
	// check correct number of command line arguments
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

	// check if port is integers
	if (isdigit(*argv[1])) {
		userPort = atoi(argv[1]); 
		printf("Using Port %i\n", userPort);
	}
	else {
		printf("Port number must be an integer value. \n");
		return 1;
	}

	//Create sockets 
	proxy_server_sock = socket(AF_INET, SOCK_STREAM, 0);
	proxy_blocker_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (proxy_server_sock == -1 || proxy_blocker_sock == -1) {
		printf("Could not create proxy server socket\n");
	}
	puts("Proxy Server Socket created.\n");

	// allow port to be reused
	if(setsockopt(proxy_server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) || 
	setsockopt(proxy_blocker_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("setsockopt\n");
		exit(EXIT_FAILURE);
	}

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(PROXY_IP); 
	server.sin_port = htons(userPort); 

	blockerLink.sin_family = AF_INET;
	blockerLink.sin_addr.s_addr = inet_addr(PROXY_IP);
	blockerLink.sin_port = htons(BLOCKER_PORT);
	
	// Bind servers
    int bindStatus = bind(proxy_server_sock, (struct sockaddr *)&server, sizeof(server));
	int bindStatus2 = bind(proxy_blocker_sock, (struct sockaddr *)&blockerLink, sizeof(blockerLink));
	if(bindStatus == -1 || bindStatus2 == -1) {
		perror("Binding failed\n");
		return 1;
	}
	printf("Binding done.\n");
	
	// Listen
	listen(proxy_server_sock, 3);
	listen(proxy_blocker_sock, 3);

	printf("Waiting for blocker to connect...\n");
	blocker_sock = accept(proxy_blocker_sock, NULL, NULL);

	// while loop to maintain connection
	while(1) { 
		printf("waiting for browser response...\n");
		int blockerStatus = fcntl(blocker_sock, F_SETFL, fcntl(blocker_sock, F_GETFL, 0) | O_NONBLOCK); // make blocker sock non-blocking

		//accept connection from an incoming client
		client_sock = accept(proxy_server_sock, NULL, NULL);
		if (client_sock < 0 || blocker_sock < 0){
			perror("Connection failed");
			return 1;
		}

		// if receive new input from blocker program, update the list accordingly
		if ((recvStatus2 = recv(blocker_sock, blockWord, 100, 0)) > 0) {
			int update = updateBlocker();
			if (update == 0) {
				break;
			}
		}
		
		// receive the GET request from the client browser
		bzero(client_message, 5000);
        recvStatus = recv(client_sock, client_message, 5000, 0);
        if (recvStatus <= 0){
            printf("nothing yet...");
        }

		// parse the HTTP request
		parseHTTP(client_message);
		
		// check for block words
		for (int i = 0; i < 100; i++) {
			if(blockList[i] != NULL && strcasestr(url, blockList[i]) != NULL) {
				path = "/~carey/CPSC441/ass1/error.html";
				printf("THIS SHOULD BE BLOCKED!\n");
			}
		}

		// get host by name
		hostStruct = gethostbyname(host);
		if (hostStruct == NULL) {
			printf("error getting host info\n");
			continue;
		}

		// make new client socket to connect to web server
		proxy_client_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (proxy_client_sock == -1) {
			printf("Could not create proxy client socket\n");
		}
		puts("Proxy Client Socket created.\n");

		// set addr and stuff to connect to web server
		bcopy(hostStruct->h_addr_list[0], &webServer.sin_addr.s_addr, hostStruct->h_length); // copy the web server addr to the client addr
		webServer.sin_family = AF_INET;
		webServer.sin_port = htons(80);

		// connect to the web server
		if(connect(proxy_client_sock, (struct sockaddr *)&webServer, sizeof(struct sockaddr_in)) < 0) {
			perror("connect failed. Error");
			return 1;
		}
		
		// construct the GET request to send to web server
		char getRequest[1048] = "GET ";
		strcat(getRequest, path);
		strcat(getRequest,"\r\n");

		// send the GET request to the web server
		if(send(proxy_client_sock, getRequest, strlen(getRequest), 0) < 0) {
			puts("Send failed\n");
			return 1;
		}

		// receive and send data from the web server to the client browser
		bzero(serverReply, sizeof(serverReply));
		int serverRecv, serverSend;
		while((serverRecv = recv(proxy_client_sock, serverReply, sizeof(serverReply), 0)) > 0) {
			if((serverSend = send(client_sock, serverReply, serverRecv, 0)) < 0) {
				puts("Send failed\n");
				return 1;
			}
			bzero(serverReply, sizeof(serverReply));
		}

		close(proxy_client_sock);	
		close(client_sock);
		free(split);  	
	} 
	
	close(proxy_server_sock);
	return 0;
}