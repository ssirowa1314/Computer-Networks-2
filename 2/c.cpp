#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/types.h>
#define PORT 3546
#define N 4
using namespace std;

void cRecv(int sockid){
	cout<<"Recv\n\n";
	int status;
	char buff[512];
	while(1){
		status = recv(sockid,buff,255,0);
		if (status < 0){
			printf("Client Error: Reading from Server");
			continue;
	    }
	   printf("\n\nBroadcast Msg: %s\n",buff);
	}
}


void cSend(int sockid){
	cout<<"Send\n\n";
	int status;
	char buff[512];
	while(1){
		bzero(buff,256);
    	printf("Write Something to Broadcast: ");
    	fgets(buff,255,stdin);

    	status = send(sockid,buff,strlen(buff),0);
		if (status < 0){
			printf("\nClient Error: Writing to Server");
		   	return;
		}
	}
}

int main(){
	int sockid, status, flag=0;
	struct sockaddr_in addrport;
	memset(&addrport, '0', sizeof(addrport)); 
	char buff[1024];
	memset(buff, '0',sizeof(buff));	

	sockid = socket(AF_INET, SOCK_STREAM, 0);
	if(sockid<0){
		printf("Client Error: Socket Creation Failed\n");
		return 0;
	}
	addrport.sin_family = AF_INET;
	addrport.sin_port = htons(PORT);

	status = inet_pton(AF_INET, "127.0.0.1", &addrport.sin_addr.s_addr);
	if(status<0)
	{
		printf("Client Error: IP not initialized succesfully");
		return 0;
	}

	status=connect(sockid, (struct sockaddr *) &addrport,sizeof(addrport));
	if (status < 0){
		printf("Client Error: Connection Failed");
		return 0;
	}
	printf("Send exit to Server to close program\n\n");
	thread tSend(cSend,sockid);
	thread tRecv(cRecv,sockid);
	tSend.join();
	tRecv.join();
	/*while(1){
    	bzero(buff,256);
    	printf("Write Something to Send to Server: ");
    	fgets(buff,255,stdin);

    	status = send(sockid,buff,strlen(buff),0);
		if (status < 0){
			printf("\nClient Error: Writing to Server");
		   	return 0;
		}
		    
		if(buff[0]=='e' && buff[1]=='x' && buff[2]=='i' && buff[3]=='t'){
			flag=1;
		}
	    bzero(buff,256);
	    status = recv(sockid,buff,255,0);
	    if (status < 0){
			printf("Client Error: Reading from Server");
			return 0;
	    }
	    printf("Message From Server: %s\n\n",buff);

	    if(flag)
	    	break;
	}*/
	close(status);
	close(sockid);
	printf("Program Closed Successfully\n");
	return 0;
}
