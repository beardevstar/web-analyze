/*
* For Test Apache Response ...
* Various Requsest from socket...
*/

#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<stdbool.h>
#include<winsock.h>
#pragma warning(disable : 4996)

#define MAX_REQU_LEN 40960
//You can confirm response data in project directory.

/*
* 
* ajax send get...
* int sended = send(sock, "GET / HTTP/1.1\r\nHost: 191.168.114.97\r\n\r\n",64,0); //success
* 
* //ajax send post...
* int sended = send(sock, "POST / HTTP/1.1\r\nHost: 191.168.114.97\r\n\r\n",64,0);//success
* 	const char* buf = "POST / HTTP/1.1\r\n"
		"Host: 192.168.114.97\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Content-Length: 21\r\n"
		"\r\nname=admin1&pwd=23092"; //success

//Form Input Text sending
	const char* buf = "POST / HTTP/1.1\r\n"
		"Host: 192.168.114.97\r\n"
		"Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryiRco4QyPQW030B8P\r\n"
		"Content-Length: 243\r\n"
		"\r\n"
		"------WebKitFormBoundaryiRco4QyPQW030B8P\r\n"
		"Content-Disposition: form-data; name=\"username\"\r\n"
		"\r\n"
		"wef\r\n"
		"------WebKitFormBoundaryiRco4QyPQW030B8P\r\n"
		"Content-Disposition: form-data; name=\"password\"\r\n"
		"\r\n"
		"wefwef\r\n"
		"------WebKitFormBoundaryiRco4QyPQW030B8P--\r\n"; //success
//File Uploading...
	const char* buf = "POST / HTTP/1.1\r\n"
		"Host: 192.168.114.97\r\n"
		"Content-Type: multipart/form-data; boundary=---------------------------273397534824044269123334396657\r\n"
		"Content-Length: 211\r\n"
		"\r\n"
		"-----------------------------273397534824044269123334396657\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"t01.c\"\r\n"
		"\r\n"
		"12345678901234567890\r\n"
		"-----------------------------273397534824044269123334396657--\r\n";
	
*/

void sendGetRuequest(int httpsock, char* _url) {
	char* sndbuf = (char*)malloc(strlen(_url) + 64);
	sprintf(sndbuf, \
		"GET %s HTTP/1.1\r\n"
		"Host: 191.168.114.97\r\n\r\n"
		, _url);
	int sended = send(httpsock, sndbuf, strlen(sndbuf), 0);
	free(sndbuf);
}

void sendPost(int httpsock,char* _url, char* _data) {
	int datalen = strlen(_data);
	char *header = (char*)malloc(128 + strlen(_url));
	sprintf(header,"POST %s HTTP/1.1\r\n"
		"Host: 192.168.114.97\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Content-Length: %d\r\n\r\n",_url, datalen);

	int totallen = strlen(header) + datalen;
	char* sndbuf = (char*)malloc(totallen + 1);
	sprintf(sndbuf, "%s%s", header, _data);
	int sended = send(httpsock, sndbuf, totallen, 0);
	free(sndbuf);
	free(header);
}

//completed
//when uploaded a file, i can't get response from server!
void sendaFile(int httpsock, char* _url, char* filepath) {
	FILE* fp = fopen(filepath,"rb"); //"rb" flag is important
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = (char*)malloc(filesize);
	int readed = fread(buffer, 1,filesize, fp);

	char boundary[64];
	sprintf(boundary, "----------%d", time(NULL));
	
	char *header1 = (char*)malloc(256 + strlen(_url));
	sprintf(header1, "POST %s HTTP/1.1\r\n"
		"Host: 192.168.114.97\r\n"
		"Content-Type: multipart/form-data; boundary=%s\r\n",_url, boundary);

	char header3[256];
	sprintf(header3, "\r\n"
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
		"\r\n", boundary, filepath);

	char tail[128];sprintf(tail,"\r\n--%s--\r\n",boundary);

	int contentlen = strlen(header3) + readed + strlen(tail);
	char header2[64];sprintf(header2, "Content-Length: %d\r\n", contentlen - 1);

	int total_len = strlen(header1) + strlen(header2) + contentlen;
	char* sendbuf = (char*)malloc(total_len);

	int pos = 0, len = strlen(header1);
	memcpy(sendbuf + pos,header1,len);
	pos += len;

	len = strlen(header2);
	memcpy(sendbuf + pos, header2, len);
	pos += len;

	len = strlen(header3);
	memcpy(sendbuf + pos, header3, len);
	pos += len;

	len = readed;
	memcpy(sendbuf + pos, buffer, len);
	pos += len;

	len = strlen(tail);
	memcpy(sendbuf + pos, tail, len);
	pos += len;
	

	FILE* fp1 = fopen("request","w");
	fwrite(sendbuf,total_len,1,fp1);
	fclose(fp1);
	
	int sended = send(httpsock, sendbuf, total_len, 0);
	free(header1);
	free(buffer);
	free(sendbuf);
	fclose(fp);
}

void* handle_request(void* socket_desc) {
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[MAX_REQU_LEN];

	//Receive a message from client	
	DeleteFileA("response");
	while ((read_size = recv(sock, client_message, MAX_REQU_LEN, 0)) > 0)
	{
		FILE* fp = fopen("response", "a");
		fwrite(client_message, read_size, 1, fp);
		fclose(fp);
	}

	if (read_size == 0)
	{
		puts("Server disconnected\n");
		fflush(stdout);
	}
	else if (read_size == -1)
	{
		puts("recv failed");
	}

	//Free the socket pointer
	closesocket(sock);
	return 0;
}

int create_socket(char* hostname, int port) {
	int sockfd;
	char      proto[6] = "";
	char* tmp_ptr = NULL;
	struct hostent* host;
	struct sockaddr_in dest_addr;

	if ((host = gethostbyname(hostname)) == NULL) {
		printf("\nError: Cannot resolve hostname %s.\n", hostname);
		exit(0);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);

	memset(&(dest_addr.sin_zero), '\0', 8);

	tmp_ptr = inet_ntoa(dest_addr.sin_addr);

	if (connect(sockfd, (struct sockaddr*)&dest_addr,
		sizeof(struct sockaddr)) == -1) {
		printf("\nError: Server May be turned off.\n#Cannot connect to host %s [%s] on port %d.\n", hostname, tmp_ptr, port);
		exit(0);
	}
	return sockfd;
}


int main(int argc, char* argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err = -1;

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		puts("WSAStartup failed with error: %d\n", err);
		exit(0);
	}
	err = -1;
	int handel_thread;
	int* new_sock = malloc(1);
	*new_sock = create_socket("127.0.0.1", 80);
	if (*new_sock == 0) return 0;
	//sendaFile(*new_sock,"/fileupload.php", "1");
	//sendPost(*new_sock, (char*)"name=admin&pwd=w234");
	//sendPost(*new_sock, "/", (char*)"name=admin234234&pwd=w234");
	sendGetRuequest(*new_sock, "/?name=admin4r3498");
	HANDLE thandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)handle_request, (void*)new_sock, 0, &handel_thread);
	WaitForSingleObject(thandle, INFINITE);
	closesocket(*new_sock);
	return 0;
}