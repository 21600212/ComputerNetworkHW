#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 4096
void error_handling(char *message);
void send_file(FILE *fp, int sock);

int main(int argc, char **argv){
	int sock;
	char message[BUFSIZE];
	int str_len, addr_size, i;
	char filename[30];
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in from_addr;

	if(argc!=4){
		printf("Usage : %s <ip> <port> <fname>\n", argv[0]);
		exit(1);
	}

	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock==-1)
		error_handling("UDP socket creating error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error!");

	strcpy(filename, argv[3]);

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		error_handling("No such file");

	printf("Sending file  %s\n", filename);	
	char *c = "*";
	strcat(filename, c);
	
	send(sock, filename, strlen(filename), 0);
	//send(sock, " ", strlen("\n"), 0);

	send_file(fp, sock);

	printf("File sending finished\n");

		
	close(sock);
	return 0;
}

void send_file(FILE *fp, int sock){
	int bytes = 0;
	char buf[BUFSIZE];

	while((bytes = fread(buf, sizeof(char), BUFSIZE, fp)) > 0){
		if(send(sock, buf, bytes, 0) < 0)
			error_handling("Send error\n");
		bzero(buf, BUFSIZE);
	}
	fclose(fp);
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}	
