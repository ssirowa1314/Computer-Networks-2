#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <climits>
#include <vector>

#define PORT_S 7598
#define PORT_C 7599
#define PAYLOADSIZE 1024
using namespace std;

vector <int> dbg;

struct sgmtHeader
{
	unsigned long long pktno;		//dbg
	unsigned long long ano;		//dbg
	int checksum;
	bool lastpkt;
	int length;
};


struct segment {
	sgmtHeader header;
	char payload[PAYLOADSIZE];
	/*segment () {
		int c=0;
		while (c<PAYLOADSIZE)
			payload[c++]='\0';
	}*/
};

int checksumFind(const sgmtHeader* packet, const char* payload) {
	int checksum = 0, c=0;
	checksum=checksum^packet->pktno^packet->ano^packet->lastpkt^packet->length;
	if(payload == NULL)
		return checksum;
	while(c<PAYLOADSIZE)
		checksum ^= payload[c++];
}

bool checksumCheck(const sgmtHeader* packet, const char* payload) {
	int x=checksumFind(packet, payload), y=packet->checksum;
	if(x==y)
		return true;
	else
		return false;
}

void sendACK(int sockid1, sgmtHeader* ack, struct sockaddr* senderAddr) {
	ack->checksum = checksumFind(ack,NULL);
	sendto(sockid1, ack, sizeof(*ack), 0, senderAddr, sizeof(*senderAddr));
}

segment* rcvSgmnt(int sockid1, unsigned long long sqno) {
	segment* packet = new segment;
	sgmtHeader* ack = new sgmtHeader;
	struct sockaddr senderAddr;
	int status ,slen=sizeof(senderAddr);
	ack->lastpkt = true;
	ack->checksum = 0;


	while (true) {
		status=recvfrom(sockid1, packet, sizeof(*packet), 0, &senderAddr, (socklen_t *)&slen);
		if (status < 0) {
			cout << "Reciever Error: Error in receiving UDP segment"<< endl;
		}
		else if (checksumCheck(&packet->header, packet->payload) == 0) {
			cout << "Reciever Error: Incorrect Checksum for UDP segment " << endl;
		}
		else if (packet->header.pktno == sqno) {
		    dbg.push_back(packet->header.pktno);
			ack->ano=sqno;
			ack->pktno=packet->header.pktno;
			sendACK(sockid1, ack, &senderAddr);
			break;
		}
		else {
			// dup ACK
			cout << "Reciever Error: Incorrect segment number for UDP segment " << endl;
			ack->pktno=packet->header.pktno;
			ack->ano=packet->header.pktno;
			sendACK(sockid1, ack, &senderAddr);
			
		}
	}
	return packet;
}

int main(int argc, char **argv) {
	int sockid1, status, total = 0, len;
	struct sockaddr_in clientAddr;
	char buffer[PAYLOADSIZE];
	ofstream ofile;
	segment* packet;
	bool state = 0;

	memset((char *)&clientAddr, 0, sizeof(clientAddr));

	if (argc != 2) {			//checking command line args
		cout<<"Invalid Command Line Arguments\n";
		return 0;
	}

	
	sockid1 = socket(AF_INET, SOCK_DGRAM, 0);			//create the socket
	if(sockid1 < 0){
		printf("Reciever Error: Socket creation unsuccessful\n");
		return 0;
	}

	
	clientAddr.sin_family = AF_INET;	// binding port to client
	clientAddr.sin_port = htons(PORT_C);
	status = inet_pton(AF_INET, "127.0.0.1", &clientAddr.sin_addr.s_addr);
	if(status < 0){
		printf("Reciever Error: IP initialization falied\n");
		return 0;
	} 

	status=bind(sockid1, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
	if (status < 0) {
		cout<<"Reciever Error: Bind Failed\n";
		return 0;
	}

	ofile.open(argv[1], ios::binary);

	while (true) {
		packet = rcvSgmnt(sockid1, total);
		cout<<"Packet "<<total<<" recvd\n";
		total=(total+1)%ULONG_MAX;
		int iter=0;
		while(iter<packet->header.length)
			ofile<<packet->payload[iter++];

		if (packet->header.lastpkt) {
			break;
		}
	}
	ofile.close();
	close(sockid1);

	for(int i=0; i<dbg.size(); i++){
		if(dbg[i]!=i){
			cout<<"\nError in File\n";
			break;
		}
	}
	cout<<"Packets sent "<<dbg.size()<<"\n";
	return 0;
}