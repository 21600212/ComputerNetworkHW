#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 4096
#define CODE200 "HTTP/1.0 200 OK\nContent-type: text/html\n\n"
#define CODE404 "HTTP/1.0 404 Not Found\nContent-type: text/html\n\n"
void error_handling(char *message);
void respond(int sock);

int main(int argc, char **argv){
	int serv_sock;
  int clnt_sock;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;

	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock==-1)
		error_handling("UDP socket creating error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

  while(1){
  	clnt_addr_size=sizeof(clnt_addr);
  	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
  	if(clnt_sock==-1)
  		error_handling("accept() error");

  	respond(clnt_sock);

  	close(clnt_sock);
  }
  close(serv_sock);
	return 0;
}

void respond(int sock){
	int bytes;
  char buf[BUFSIZE];
	FILE *fp;
  char *method;
  char *uri;

  unsigned char html[BUFSIZE];
  unsigned char header[1024];


	bytes = recv(sock, buf, BUFSIZE, 0);
  buf[bytes] = '\0';
  printf("%s", buf);

  method = strtok(buf,  " ");
  uri = strtok(NULL, " ") + 1;

  printf("method: %s\n", method);
  printf("uri: %s\n", uri);
  if(uri[0] == 0) strcat(uri, "index.html");

  if(strcmp(method, "GET") == 0){
    fp = fopen(uri, "rb");
    if (fp == NULL)
      strcpy(header,CODE404);
    else
      strcpy(header,CODE200);

    send(sock, header, strlen(header), 0);

  	bzero(buf, BUFSIZE);
  	bytes = 0;

    if (fp != NULL)
      while((bytes = fread(buf, sizeof(char), BUFSIZE, fp)) > 0){
    		if(send(sock, buf, bytes, 0) < 0)
    			error_handling("Send error\n");
    		bzero(buf, BUFSIZE);
    	}
  }
  else if(strcmp(method, "POST") == 0){
    strcpy(header,CODE200);
    send(sock, header, strlen(header), 0);

    char *temp = strtok(NULL, "\n");
    char *next = temp + 1 + strlen(temp);
    while(1){
      if(next[1]=='\n') break;
      temp = strtok(NULL, "\n");
      next = temp + 1 + strlen(temp);
    }
    temp = next + 2;

    strcpy(html,
        "<!DOCTYPE html>\n"
        "<html lang = \"ja\">\n"
        "<head>\n"
        "<meta charset = \"utf-8\">\n"
        "</head>\n"
        "<body>\n<h2>");

    if(send(sock, html, strlen(html), 0) < 0)
      error_handling("Send error\n");

    if(send(sock, temp, strlen(temp), 0) < 0)
      error_handling("Send error\n");

    strcpy(html,
        "</h2></body>"
        "</html>");

    if(send(sock, html, strlen(html), 0) < 0)
      error_handling("Send error\n");
  }

	fclose(fp);

	return;
}


void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
