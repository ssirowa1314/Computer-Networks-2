#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <climits>
#include <chrono>
#include <math.h>

#define PORT_S 7598
#define PORT_C 7599
#define PAYLOADSIZE 1024
#define K 500
using namespace std;

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

bool rACK(int sockid1, unsigned long long pktno, int xx=0) {
	struct sockaddr serverAddr;
	sgmtHeader* ack = new sgmtHeader;
	int slen = sizeof(serverAddr), status;

	status=recvfrom(sockid1, ack, sizeof(*ack), 0, &serverAddr, (socklen_t *)&slen);
	if (status < 0) {
		cout << "Sender Error: in receiving UDP ACK\n";
		return false;
	}
	else if(ack->ano != pktno){
		cout<<"Sender Error: Incorrect ACK recieved\n";
		return false;
	}
	else if (checksumCheck(ack,NULL) == false) {
		cout << "Sender Error: Incorrect Checksum for UDP ACK"<<endl;
		return false;
	}

	return true;
}


void bindPorts(sockaddr_in &serverAddr, sockaddr_in &clientAddr, int &sockid1){
	int status;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(PORT_C);
	status = inet_pton(AF_INET, "127.0.0.1", &clientAddr.sin_addr.s_addr);
	if(status < 0){
		printf("Client:IP initialization falied\n");
		return;
	} 


	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_S);
	status = inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr.s_addr);
	if(status < 0){
		printf("Server:IP initialization falied\n");
		return;
	} 
	status=bind(sockid1, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	if(status < 0) {
		printf("Server:Bind failed\n");
		exit(0);
	}
}


void sendUDPF(int sockid1, segment* packet, struct sockaddr* destAddr, unsigned long long xx=-1) {
	int status, cs=checksumFind(&packet->header, packet->payload);
	unsigned long long c=0;
	packet->header.checksum =cs;
	while (true) {
		c++;
		status=sendto(sockid1, packet, sizeof(*packet), 0, destAddr, sizeof(*destAddr));
		if (status < 0) {
			cout << "Sender Error: in sending UDP frame"<< endl;
		}
		else {
			if (rACK(sockid1,packet->header.pktno) == true) {
				break;
			}
			cout << "Sender Error: Time out !!!!!\n" << endl;
		}

		if(c>K && packet->header.pktno==xx){
			break;
		}
	}
	//cout<<packet->header.pktno<<" times "<<c<<"\n";
}

int main(int argc, char **argv) {
	int sockid1, status;
	struct sockaddr_in serverAddr, clientAddr;
	struct timeval timer;
	struct stat fStatus;
	unsigned int c_sz = 0, f_sz;	
	unsigned long long total_pkt;
	char buff[PAYLOADSIZE];
	int total = 0;
	segment* packet = new segment;
	bool state = 0;
	ifstream ifile;
	ofstream ofile;

	timer.tv_sec = 0;
	timer.tv_usec = 40000;

	memset((char *)&clientAddr, 0, sizeof(clientAddr));
	memset((char *)&serverAddr, 0, sizeof(serverAddr));

	if (argc != 2) {								//checking command line args
		cout<<"Sender Error: Invalid Command Line Arguments\n";
		return 0;
	}

	sockid1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);		//creating socket for server
	if(sockid1 < 0){
		printf("Sender Error: Socket creation unsuccessful\n");
		return 0;
	}

	status=setsockopt(sockid1, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));
	if (status < 0) {
		cout << "Server: Error Creating Timeout " << endl;
		return 0;
	}

	// binding port to server and client
	bindPorts(serverAddr,clientAddr, sockid1);
	
	stat(argv[1], &fStatus);
	f_sz = fStatus.st_size;
	ifile.open(argv[1], ios::binary);

	ofile.open("time.txt", ios::app);
	total_pkt=ceil(f_sz/PAYLOADSIZE);
	cout << "Sending File " <<argv[1]<<" of "<<f_sz << " bytes. Total packets to be sent "<<total_pkt<<"\n";

	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	while (ifile.eof() == false) {
		ifile.read(packet->payload, PAYLOADSIZE);
		packet->header.ano = 0;
		packet->header.checksum = 0;
		packet->header.pktno=total%ULONG_MAX;

		c_sz += PAYLOADSIZE;			//current size

		if (c_sz < f_sz) {			
			packet->header.lastpkt = false;
			packet->header.length = PAYLOADSIZE;
		}
		else {
			packet->header.lastpkt = true;
			packet->header.length = f_sz - c_sz + PAYLOADSIZE;
		}

		sendUDPF(sockid1, packet, (struct sockaddr*) (&clientAddr),total_pkt-1);
		cout<<"Packet "<<++total<<" sent\n";
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	cout << "total packets sent :: " << total << endl;

	std::chrono::duration<double> elapsed_seconds = end-start;
	cout << "Time difference = " << elapsed_seconds.count() <<"s\n";
	ofile<< "Time difference = " << elapsed_seconds.count() <<"s ";
	
	ofile<<"\n";
	ifile.close();
	ofile.close();
	close(sockid1);

	return 0;
}