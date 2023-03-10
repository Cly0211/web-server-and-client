/*
 * name: Liyi Chen
 * Case ID: lxc596
 * file name: proj2.c
 * Created on 9/16/2022
 * This file implements -u, -i, -c, -s, -o, and -f options.
 * This file can send request to a web server and receive response.It also can print req and rsp if required.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
char hostName[100];
char fileName[100];
char localFileName[100];
//"-u" option
int uOption(char url[]){
    //check if the URL begins with http://
    if (strncasecmp(url, "http://",7)!=0){
        printf("error missing URL(URL should begin with http://)");
	return -1;
    }
    //find the first '/'
    int first = 7;
    while (first < strlen(url) & url[first] != '/'){
        first++;
    }
    //find hostname
    int i = 0;
    for (i = 0; i < first-7; i++){
        hostName[i] = url[i+7];
    }
    hostName[first-7] = '\0';
    //find filename
    if (first == strlen(url)){
        fileName[0]='/';
        fileName[1]='\0';
    }
    else{
        int j = 0;
        for (j = first; j < strlen(url)+1; j++){
            fileName[j-first] = url[j];
        }
        fileName[strlen(url)-first] = '\0';
    }
    return 0;
}

//"-i" option
int iOption(){
    //printf("detect -i\n");
    printf("INF: hostname = %s\n",hostName);
    printf("INF: web_filename = %s\n", fileName);
    printf("INF: output_filename = %s\n", localFileName);
    return 0;
}

//"-c" option
int cOption(){
    //printf("detect -c\n");
    printf("REQ: GET %s HTTP/1.0\r\nREQ: Host: %s\r\nREQ: User-Agent: CWRU CSDS 325 Client 1.0\n", fileName, hostName);
    return 0;}

//get IP by hostname
char * hostToIp(char *hostname){
    struct hostent *host_entry = gethostbyname(hostname);
    
    if(host_entry){
        return inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);
    }
    return NULL;
}

//create socket
//use TCP protocol
int createSocket(char *ip){
    //create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //set ip
    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr(ip);
    //connect
    //return 0 if successful
    if(0 != connect(sockfd, (struct sockaddr*)&sin, sizeof(struct sockaddr_in)))
    {
        return -1;
    }
    return sockfd;
}

//-s, -o, -f options
int sendReq(char *hostname, char *filename, int s, int f, int i, int c){
    char *ip = hostToIp(hostname);
    int sockfd = createSocket(ip);
    char buffer[1024],buffer2[1024];

    memset(buffer,0x00,sizeof(buffer));
    memset(buffer2,0x00,sizeof(buffer2));

    sprintf(buffer, "GET %s HTTP/1.0\r\n",fileName);
    sprintf(buffer2,"Host: %s\r\n",hostName);
    strcat(buffer,buffer2);
    strcat(buffer,"User-Agent: CWRU CSDS 315 Client 1.0\r\n");
    strcat(buffer,"\r\n");

    //send request
    send(sockfd, buffer, strlen(buffer), 0);

    //receive response
    FILE *sp;
    char line[1025];
    int ok_rsp = 0;
    int moved_rsp = 0;
    char location[200];
    sp = fdopen(sockfd,"r");
    memset(line,0,sizeof(line));
    fgets(line,1024,sp);
    if (s == 1)
    	printf("RSP: %s",line);
    if (strncasecmp(line,"HTTP/1.1 200",12)==0 || strncasecmp(line,"HTTP/1.0 200",12)==0)
	    ok_rsp = 1;
    if (strncasecmp(line,"HTTP/1.1 301",12)==0 || strncasecmp(line, "HTTP/1.0 301",12)==0)
	    moved_rsp = 1;
    while(1){
	    memset(line,0,sizeof(line));
	    fgets(line,1024,sp);
	    if (strcmp(line,"\r\n")==0)
		    break;
	    if (s == 1)
	    	    printf("RSP: %s",line);
	    if (f == 1 & moved_rsp == 1 & strncasecmp(line,"Location: ",10)==0){
		    strcpy(location,line);
		    int j = 0;
		    for (j = 0; j <strlen(location)-10; j++ ){
			    location[j] = location[j+10];
		    }
		    location[j-2] = '\0';
	    }
    }
    if (f == 1 & moved_rsp == 1){
            uOption(location);
	    if (i == 1)
	            iOption();
	    if (c == 1)
                    cOption();
	    return sendReq(hostName,fileName,s,f,i,c);
    }
    if (ok_rsp != 1){
	    printf("non-200 response");
	    return -1;
    }
    FILE *ptrFile = fopen(localFileName, "w");
    while(!feof(sp)){
  	    memset(line,0,sizeof(line));
	    fread(line,sizeof(char),1024,sp);
  	    fwrite(line,sizeof(char),1024,ptrFile);
    }
    fclose(sp);
    fclose(ptrFile);
   return 0;   
}

int main(int argc, char *argv[], char *envp[]){
int u = 0,i = 0, c = 0, s = 0, o = 0, f = 0;
    char url[100];
    for (int j = 1; j < argc; j++){
        if (strcmp(argv[j], "-u" )==0){
            //check if the URL exists in command
            if (j+1 >= argc){
                printf("missing URL (URL should begin with http://)\n");
                return -1;
            }
            strcpy(url,argv[j+1]);
            u++;
            j++;

        }
        else if (strcmp(argv[j], "-o" )==0){
            //check if the filename exists in command
            if (j+1 >= argc){
                printf("missing filename\n");
                return -1;
            }
            o++;
            strcpy(localFileName,argv[j+1]);
            j++;
        }
        else if (strcmp(argv[j], "-i" )==0){
            i++;
        }
        else if (strcmp(argv[j], "-c" )==0){
            c++;
        }
        else if (strcmp(argv[j], "-s" )==0){
            s++;
        }
	else if (strcmp(argv[j], "-f")==0){
	    f++;
	}
        else{
            printf("unknown argument\n");
            return -1;
        }
    }
    if (u<1){
        printf("missing -u option\n");
        return -1;
    }
    if (o<1){
        printf("missing -o option\n");
        return -1;
    }
    uOption(url);
    if (i == 1){
        iOption();
    }
    if (c == 1){
        cOption();
    }
    sendReq(hostName,fileName,s,f,i,c);
    return 0;
}

