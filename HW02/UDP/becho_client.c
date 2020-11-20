#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 30
void error_handling(char *message);
void send_filename(char filename[30], int sock);
void send_file(FILE *fp, int sock);

int main(int argc, char **argv){
	int sock;
	char buf[BUFSIZE];
	int bytes, addr_size, i;
	char filename[30];

	struct timeval timeout = {5, 0};
	struct sockaddr_in serv_addr;
	struct sockaddr_in from_addr;

	if(argc!=4){
		printf("Usage : %s <ip> <port> <fname>\n", argv[0]);
		exit(1);
	}

	sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(sock==-1)
		error_handling("UDP socket creating error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	strcpy(filename, argv[3]);

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		error_handling("No such file");
	
	
	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		error_handling("set sock time out opt failed.\n");

	char checksum[60];
	strcpy(checksum, filename);
	strcat(checksum, " ");
	strcat(checksum, filename);

	// Send file name
	printf("Sending File Name\n");		
	while(1)
	{
		sendto(sock, checksum, strlen(checksum), 0, 
			(struct sockaddr*)&serv_addr, sizeof(serv_addr));
		
		addr_size = sizeof(from_addr);
		bytes = recvfrom(sock, buf, BUFSIZE, 0,
			(struct sockaddr*)&from_addr, &addr_size);
		
		if(bytes == -1)
			printf("recv time out. Trying it again\n");
		else
		{
			printf("Server got file name\n", buf);
			break;
		}
	}

	// Send file contents
	printf("Sending File Contents\n");
	while((bytes = fread(buf, sizeof(char), BUFSIZE, fp)) > 0){
		bzero(buf, BUFSIZE);
		sendto(sock, buf, bytes, 0, 
			(struct sockaddr*)&serv_addr, sizeof(serv_addr));
	}

	fclose(fp);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}	
