#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 4096
void error_handling(char *message);
void write_file(int sock);

int main(int argc, char **argv){
	int serv_sock, clnt_sock;
	char message[BUFSIZE];
	int str_len, num=0;
	
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

	clnt_addr_size=sizeof(clnt_addr);
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");
	
	write_file(clnt_sock);
	
	close(clnt_sock);
	return 0;
}

void write_file(int sock){
	int bytes;
	FILE *fp;
	char buf[BUFSIZE];
	char filename[30];

	bytes = recv(sock, buf, BUFSIZE, 0);

	char *ptr = strtok(buf, " ");
	strcpy(filename, ptr);

	fp = fopen(filename, "wb");
	printf("filename arrived: %s\n", filename);

	ptr = strtok(NULL, " ");
	while (ptr != NULL)
	{
		fwrite(ptr, sizeof(char), sizeof(ptr), fp);
		ptr = strtok(NULL, " ");
	}
	
	bzero(buf, BUFSIZE);
	bytes = 0;
	
	while(bytes = recv(sock, buf, BUFSIZE, 0)) {
		if (bytes < 0)
			error_handling("Receive Error\n");
		if (fwrite(buf, sizeof(char), bytes, fp) < bytes)
			error_handling("File Writing Error\n");
		bzero(buf, BUFSIZE);
	}
	fclose(fp);
	printf("File %s is received.\n", filename);

	return;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}	
