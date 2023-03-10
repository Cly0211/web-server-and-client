/*
 * Liyi Chen
 * lxc596
 * filename: proj3.c
 * created on 10/14/2022
 * This class works as a server which can response to requests with GET and TERMINATE methods
 */
#include<stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>

int main(int argc, char *argv[], char *envp[]){
	int port = 0;
	char document_directory[100];
	char auth_token[100];
	int p = 0, r = 0, t = 0;
	if (argc!=7){
		printf("unknown or missing commands\n");
		return -1;
	}
	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i], "-p")==0){
			p++;
			if (i+1>=argc){
				printf("missing port number\n");
				return -1;
			}
			port=atoi(argv[i+1]);
			if (port<1025 || port>65535){
				printf("the port number should be between 1025 and 65535\n");
				return -1;
			}
			i++;
		}
		else if (strcmp(argv[i],"-r")==0){
			r++;
			if (i+1>=argc){
				printf("missing directory\n");
				return -1;
			}
			strcpy(document_directory,argv[i+1]);
			struct stat s;
			if (stat(document_directory,&s)!=0 || !(s.st_mode & S_IFDIR)){
				printf("not a directory\n");
				return -1;
			}
			i++;
		}
		else if (strcmp(argv[i],"-t")==0){
			t++;
			if (i+1>=argc){
				printf("missing authentication token\n");
				return -1;
			}
			strcpy(auth_token,argv[i+1]);
			i++;
		}
	}
	if (p<1){
		printf("missing -p option\n");
		return -1;
	}
	if (r<1){
		printf("missing -r option\n");
		return -1;
	}
	if (t<1){
		printf("missing -t option\n");
		return -1;
	}
	
	int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (listenfd < 0){
		printf("socket error\n");
		return -1;
	}
	
	struct sockaddr_in svraddr;
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(port);
	svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd ,(struct sockaddr *)&svraddr,sizeof(svraddr)) < 0){	
		printf("bind error\n");
		return -1;
	}

	if (listen(listenfd,20)<0){
		printf("listen error\n");
		return -1;
	}

	while(1){
		struct sockaddr_in cliaddr;
		memset(&cliaddr,0,sizeof(cliaddr));
		socklen_t ret = sizeof(cliaddr);
		int sockfd = accept(listenfd,(struct sockaddr *)&cliaddr, &ret);

		if(sockfd<0){
			printf("accept error\n");
			return -1;
		}

		FILE *sp;
		char buffer[1025];
		memset(buffer,0x00,sizeof(buffer));
		char rspBuffer[1025];
		memset(rspBuffer,0x00,sizeof(rspBuffer));
		sp = fdopen(sockfd,"r");
		
		fgets(buffer,1024,sp);
		if (buffer[strlen(buffer)-2]!='\r' || buffer[strlen(buffer)-1]!='\n'){
			memset(rspBuffer,0x00,sizeof(rspBuffer));
			sprintf(rspBuffer,"HTTP/1.1 400 Malformed Request\r\n\r\n");
			send(sockfd,rspBuffer,strlen(rspBuffer),0);
			close(sockfd);
			continue;
		}
		 //split the first line into content[3],
                //in which content[0] stores the method,
                //content[1] stores the argument,
                //content[2] stores the http version
		char content[3][100]={0};
		int i = 0;
		while(buffer[i] != ' ' & i<strlen(buffer)){
			i++;
		}
		if (i >= strlen(buffer)-1){
			memset(rspBuffer,0x00,sizeof(rspBuffer));
			sprintf(rspBuffer,"HTTP/1.1 400 Malformed Request\r\n\r\n");
			send(sockfd,rspBuffer,strlen(rspBuffer),0);
			close(sockfd);
			continue;
		}
		for (int j = 0; j < i; j++){
			content[0][j] = buffer[j];
		}
		int k = i+1;
		while(buffer[k] != ' ' & k<strlen(buffer)){
			k++;
		}
		if (k >= strlen(buffer)-1){
			memset(rspBuffer,0x00,sizeof(rspBuffer));
			sprintf(rspBuffer,"HTTP/1.1 400 Malformed Request\r\n\r\n");
			send(sockfd,rspBuffer,strlen(rspBuffer),0);
			close(sockfd);
			continue;
		}
		for (int j = 0; j < k-i-1; j++){
			content[1][j] = buffer[j+i+1];
		}
		int l = k+1;
		while(buffer[l] != ' ' & l<strlen(buffer)){
			l++;
		}
		if (l != strlen(buffer)){
			memset(rspBuffer,0x00,sizeof(rspBuffer));
			sprintf(rspBuffer,"HTTP/1.1 400 Malformed Request\r\n\r\n");
			send(sockfd,rspBuffer,strlen(rspBuffer),0);
			close(sockfd);
			continue;
		}
		for (int j = 0; j < l-k-1; j++){
			content[2][j] = buffer[j+k+1];
		}
		
		while(1){
			memset(buffer,0,sizeof(buffer));
			fgets(buffer,1024,sp);
			if (buffer[strlen(buffer)-2]!='\r' || buffer[strlen(buffer)-1]!='\n'){
				memset(rspBuffer,0x00,sizeof(rspBuffer));
				sprintf(rspBuffer,"HTTP/1.1 400 Malformed Request\r\n\r\n");
				send(sockfd,rspBuffer,strlen(rspBuffer),0);
				close(sockfd);
				continue;
			}
			if (strcmp(buffer,"\r\n")==0){
				break;
			}
		}

		//if no protocal
		if (strncmp(content[2],"HTTP/",5)!=0){
			memset(rspBuffer,0x00,sizeof(rspBuffer));
			sprintf(rspBuffer,"HTTP/1.1 501 Protocal Not Implemented\r\n\r\n");
			send(sockfd,rspBuffer,strlen(rspBuffer),0);
			close(sockfd);
			continue;
		}

		//if TERMINATE method
		if (strcmp(content[0],"TERMINATE")==0){
			if (strcmp(content[1],auth_token)==0){
				memset(rspBuffer,0x00,sizeof(rspBuffer));
				sprintf(rspBuffer,"HTTP/1.1 200 Server Shutting Down\r\n\r\n");
				send(sockfd,rspBuffer,strlen(rspBuffer),0);
				close(sockfd);
				close(listenfd);
				return 0;
			}
			memset(rspBuffer,0x00,sizeof(rspBuffer));
			sprintf(rspBuffer,"HTTP/1.1 403 Operation Forbidden\r\n\r\n");
			send(sockfd,rspBuffer,strlen(rspBuffer),0);
			close(sockfd);
			continue;
		}
		//if GET method
		else if (strcmp(content[0],"GET")==0){
			if (strncmp(content[1],"/",1)!=0){
				memset(rspBuffer,0x00,sizeof(rspBuffer));
				sprintf(rspBuffer,"HTTP/1.1 406 Invalid FileName\r\n\r\n");
				send(sockfd,rspBuffer,strlen(rspBuffer),0);
				close(sockfd);
				continue;
			}
			if (strcmp(content[1],"/")==0)
				strcpy(content[1],"/homepage.html");
			strcat(document_directory,content[1]);
			FILE *fp = fopen(document_directory,"r");
			if (fp == NULL){
				memset(rspBuffer,0x00,sizeof(rspBuffer));
				sprintf(rspBuffer,"HTTP/1.1 404 File Not Found\r\n\r\n");
				send(sockfd,rspBuffer,strlen(rspBuffer),0);
				close(sockfd);
				continue;
			}
			sprintf(rspBuffer,"HTTP/1.1 200 OK\r\n\r\n");
			send(sockfd,rspBuffer,strlen(rspBuffer),0);
			while(!feof(fp)){
				memset(rspBuffer,0,sizeof(rspBuffer));
				int rlen = fread(rspBuffer,sizeof(char),1024,fp);
				send(sockfd,rspBuffer,rlen,0);
			}
			close(sockfd);
			continue;
		}
		//unknown methods
		else{
			memset(rspBuffer,0x00,sizeof(rspBuffer));
			sprintf(rspBuffer,"HTTP/1.1 405 Unsupported Method\r\n\r\n");
			send(sockfd,rspBuffer,strlen(rspBuffer),0);
			close(sockfd);
			continue;
		}
	close(sockfd);	
	}
	close(listenfd);
	return 0;

}

