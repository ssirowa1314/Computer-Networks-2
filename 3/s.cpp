#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/types.h>
#include <iostream>
#define PORT 3541
#define N 2
using namespace std;

vector <int> clientList;


struct pkt{
	int fname;
	int num;
	int arg3[4];
	float arg12[11];
};


void newConn(int index){
	int n;
	pkt buff;

	while(1){
		n=recv(clientList[index], &buff, sizeof(buff),0);

		if(n<0){
			printf("Server:Receive Error\n");
			return;
		}

		if(buff.fname == 1){
			buff.arg12[10]=sqrt(buff.arg12[0]);
		}
		else if(buff.fname == 2){
			buff.arg12[10]=0;
			for(int i=0; i<buff.num; i++){
				buff.arg12[10]+=buff.arg12[i];
			}
		}
		else if(buff.fname == 3){
			cout<<buff.arg3[0]<<" "<<buff.arg3[1]<<"\n";
			buff.arg3[2]=buff.arg3[0]/buff.arg3[1];
			buff.arg3[3]=buff.arg3[0]%buff.arg3[1];
		}
		else{
			close(clientList[index]);
			return;
		}
		send(clientList[index], &buff, sizeof(buff), 0);
	}

	//send(clientList[i],)
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

    while(1){
    	clen=sizeof(clientAddr);
		sockid2=accept(sockid1,(struct sockaddr*)&clientAddr, (socklen_t*)&clen);	//check here

		if(sockid2<0){
			printf("Server:Socket Creation failed for accept command\n");
			return 0;
		}

		clientList.push_back(sockid2);
		//cout<<sockid2<<"\n";
		client.emplace_back(newConn,clientList.size()-1);

    }

    for (auto& v : client){
        v.join();
    } 

    close(sockid1);
    return 0;
}