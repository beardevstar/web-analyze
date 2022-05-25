/*
* For Capturing Web Browsers Request Headers and Content.
* call localhost:8000 or In Form set action=http://localhost:8000 and submit
*/

#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<stdbool.h>
#include<winsock.h>
#pragma warning(disable : 4996)

#define PORT_NO         8000
#define INVALID_SOCK    -1
#define MAX_BACK_LOG    5
#define MAX_LEN         256
#define MAX_REQU_LEN    (MAX_LEN+16+16)*128
#define SPACE_CH    	' '
#define LINE_CH 	'\n'


/**
 * ./server .
 * localhost:8000/index.php
 * gcc server.c -o server -lpthread
 *
 **/

char* rootpath;
typedef struct _REQUEST {
	char tp[16];
	char path[MAX_LEN];
	char version[16];
}H_REQUEST;

bool parse_request(char* _msg, H_REQUEST* result) {
	if (_msg == NULL || strncmp(_msg, "GET", 3) != 0)
		return false;
	char* pos = strchr(_msg, LINE_CH);
	char* tp = strchr(_msg, SPACE_CH);
	if (pos == NULL || tp == NULL) return false;

	strncpy(result->tp, _msg, tp - _msg);

	char* path_start = tp + 1;
	tp = strchr(path_start, SPACE_CH);
	if (tp == NULL) return false;

	strncpy(result->path, path_start, tp - path_start);
	strncpy(result->version, tp, pos - tp);
	return true;
}

bool send_response(int _cli_sock, char* _content, int _clen) {
	unsigned int code = 200;
	if (_content == NULL) {
		_content = "Not Found";
		code = 404;
	}
	int sock = _cli_sock;
	char response[MAX_REQU_LEN];
	sprintf(response, "HTTP/1.0 %d OK\nContent-Length:%ld\ncontent-type:text/html; charset=UTF-8\n\n%s", code, strlen(_content), _content);
	write(sock, response, strlen(response));
	puts(response);
	return true;
}

void* handle_request(void* socket_desc) {
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[MAX_REQU_LEN];

	//Receive a message from client	
	DeleteFileA("request");
	while ((read_size = recv(sock, client_message, MAX_REQU_LEN,0)) > 0)
	{
		FILE* fp = fopen("request", "a");
		fwrite(client_message,read_size,1,fp);
		fclose(fp);
	}
	
	if (read_size == 0)
	{
		puts("Client disconnected\n");
		fflush(stdout);
	}
	else if (read_size == -1)
	{
		perror("recv failed");
	}

	//Free the socket pointer
	closesocket(sock);
	return 0;
}

int main(int argc, char* argv[])
{

	WORD wVersionRequested;
	WSADATA wsaData;
	int err = -1;

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		printf("WSAStartup failed with error: %d\n", err);
		exit(0);
	}
	err = -1;

	int     socket_desc, cli_sock, sock_len, * new_sock;
	struct sockaddr_in server, clinfo;
	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == INVALID_SOCK)
		printf("Could not create socket");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT_NO);

	//Bind
	if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		puts("bind failed");
		return 1;
	}
	//Listen
	listen(socket_desc, MAX_BACK_LOG);    //backlog max
	puts("waiting for client's connections.");
	//Accept and incoming connection
	sock_len = sizeof(struct sockaddr_in);
	while ((cli_sock = accept(socket_desc, (struct sockaddr*)&clinfo, (int*)&sock_len)))
	{
		puts("Connection accepted...");

		int handel_thread;
		new_sock = malloc(1);
		*new_sock = cli_sock;
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)handle_request, (void*)new_sock, 0, &handel_thread);
		//pthread_create(&handel_thread, NULL, handle_request, (void*)new_sock);
	}

	if (cli_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	close(socket_desc);
	return 0;
}