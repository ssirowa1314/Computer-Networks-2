#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#define PORT 3541
using namespace std;

int main(){
	int sockid1, sockid2, status, clen, n, flag=0;
	char buff[1024];
	memset(buff, '0',sizeof(buff));	

	struct sockaddr_in addrport, clientAddr;
	memset(&addrport, '0', sizeof(addrport)); 
	memset(&clientAddr, '0', sizeof(clientAddr));

	sockid1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockid1 < 0){
		printf("Server:Socket creation unsuccessful\n");
		return 0;
	}

	addrport.sin_family = AF_INET;	
	addrport.sin_port = htons(PORT);

	status = inet_pton(AF_INET, "127.0.0.1", &addrport.sin_addr.s_addr);
	if(status < 0){
		printf("Server:IP initialization falied\n");
		return 0;
	}  

	status=bind(sockid1, (struct sockaddr *) &addrport, sizeof(addrport));
	if(status < 0) {
		printf("Server:Bind failed\n");
		return 0;
	}

	status=listen(sockid1,1);
	if(status < 0){
		printf("Server:Listen failed\n");
		return 0;
	}

	clen=sizeof(clientAddr);
	sockid2=accept(sockid1,(struct sockaddr*)&clientAddr, (socklen_t*)&clen);	//check here
	if(sockid2<0){
			printf("Server:Socket Creation failed for accept command\n");
			return 0;
	}

	printf("Program will close upon receiving exit from Client\n\n");
	while (1)  
	{
		bzero(buff,256);
		n=recv(sockid2, buff, 256,0);
		if(n<0){
			printf("Server:Receive Error\n");
			return 0;
		}

		printf("Message from Client: %s\n",buff);

		if(buff[0]=='e' && buff[1]=='x' && buff[2]=='i' && buff[3]=='t'){
			flag=1;
		}

		send(sockid2, "Server Recived Your Message", strlen("Server Recived Your Message"), 0);
		printf("\n");

		if(flag)
			break;
										
	}
	close(sockid1);
	close(sockid2);
	printf("Program Closed Successfully\n");

	return 0;
}

