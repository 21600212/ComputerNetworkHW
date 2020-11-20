#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 500
#define NAMESIZE 30
void error_handling(char *message);

int main(int argc, char **argv){
	int serv_sock;
	char buf[BUFSIZE];
	char filename[NAMESIZE];
	char checksum[NAMESIZE];
	int bytes, num=0;
	char *ptr;

	struct timeval timeout = {5, 0};
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;

	int clnt_addr_size;

	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock==-1)
		error_handling("UDP socket creating error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");


	// Recieve file name
	while(1){
		printf("Waiting for file name\n");	
		clnt_addr_size=sizeof(clnt_addr);
		bytes = recvfrom(serv_sock, buf, BUFSIZE, 0,
			(struct sockaddr*)&clnt_addr, &clnt_addr_size);
		
		bzero(filename, NAMESIZE);
		bzero(checksum, NAMESIZE);

		ptr = strtok(buf, " ");
		strcpy(filename, ptr);
		ptr = strtok(NULL, " ");
		strcpy(checksum, ptr);
	
		printf("Cmp [%s] vs [%s]\n", filename, checksum);
		if(strcmp(filename, checksum) == 0)
		{
			printf("File name arrived without Loss\n");
			if (setsockopt (serv_sock, SOL_SOCKET, SO_RCVTIMEO, 
				(char *)&timeout, sizeof(timeout)) < 0)
				error_handling("set sock time out opt failed.\n");

			while(1)
			{
				printf("Sending ACK to Client\n");
				sendto(serv_sock, filename, sizeof(filename), 0,
					(struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
				bzero(buf, BUFSIZE);
				bytes = recvfrom(serv_sock, buf, BUFSIZE, 0,
					(struct sockaddr*)&clnt_addr, &clnt_addr_size);
				if(bytes != -1) break;
			}
			break;
		}
		printf("File Name Loss Occured.\n");
		bzero(buf, BUFSIZE);
	}

	// Recieve file content
	printf("Saving file contents\n");

	FILE *fp = fopen(filename, "wb");
	fwrite(buf, sizeof(char), bytes, fp);

	while(1){
		bzero(buf, BUFSIZE);
		clnt_addr_size=sizeof(clnt_addr);
		bytes = recvfrom(serv_sock, buf, BUFSIZE, 0,
			(struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if (bytes == -1) break;
		fwrite(buf, sizeof(char), bytes, fp);
	}
	fclose(fp);
	printf("File saving finished\n");
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}	
