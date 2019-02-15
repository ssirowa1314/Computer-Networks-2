#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/types.h>
#include <iostream>
#define PORT 3546
#define N 5
using namespace std;

vector <int> clientList;
queue <string> msg;
queue <int> msgI;

void fSend(int index){
	string temp;
	char buff[512];
	int c;
	cout<<"Send  "<<index<<"\n";
	while(1){
		if(msg.size() == 0)
			continue;
		temp=msg.front();
		c=msgI.front();
		msgI.pop();
		msg.pop();
		bzero(buff,256);
		strcpy(buff,temp.c_str());
		for(int i=0; i<N; i++){
			if(c==i)
				continue;
			send(clientList[i], buff, strlen(buff), 0);
		}
	}
}


void newConn(int index){
	//cout<<"Recv  "<<clientList[index]<<"\n";
	cout<<"Recv  \n";
	int n;
	char buff[512];
	while(1){
		bzero(buff,256);
		n=recv(clientList[index], buff, 256,0);
		if(n<0){
			printf("Server:Receive Error\n");
			return;
		}
		printf("%s\n",buff);
		msg.push(buff);
		msgI.push(index);
	}
}

int main(){
	int sockid1, sockid2, status, clen, n, flag=0;
	//string buff;

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

	status=listen(sockid1,N);
	if(status < 0){
		printf("Server:Listen failed\n");
		return 0;
	}

	
	vector <thread> client;

	thread cSend(fSend,0);

    while(1){
    	clen=sizeof(clientAddr);
		sockid2=accept(sockid1,(struct sockaddr*)&clientAddr, (socklen_t*)&clen);	//check here
		if(sockid2<0){
			printf("Server:Socket Creation failed for accept command\n");
			return 0;
		}
		clientList.push_back(sockid2);
		cout<<sockid2<<"\n";
		client.emplace_back(newConn,clientList.size()-1);

    }
    /*int ctr=0;
    for (auto& v : client){

    }*/
}