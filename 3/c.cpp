#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/types.h>
#define PORT 3541
#define N 50
using namespace std;

struct pkt{
	int fname;
	int num;
	int arg3[4];
	float arg12[11];
};


int main(){
	int sockid, status, flag=0;
	struct sockaddr_in addrport;
	memset(&addrport, '0', sizeof(addrport)); 

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
	
	pkt buff;
	int choice=0;
	//buff.fname=1;
	while(choice != 4){
		choice=0;
		cout<<"Enter your choice of Function:\n1) Press 1 for calculating sqrt\n2)Press 2 for calculating add\n3)Press 3 for calulating remainder and quotient in division\n4) Press 4 to quit program\n";
		while(choice<1 || choice>4){
			cout<<"Enter your choice: ";
			cin>>choice;
		}
		buff.fname=choice;

		switch(choice){
			case 1: cout<<"Enter the argument of sqrt: ";
					cin>>buff.arg12[0];

					status = send(sockid,&buff,sizeof(buff),0);
					if (status < 0){
						printf("\nClient Error: Writing to Server");
		   				return 0;
					}
					status = recv(sockid,&buff,sizeof(buff),0);
					if (status < 0){
						printf("Client Error: Reading from Server");
						return 0;
	   				}
	   				cout<<"Result is "<<buff.arg12[10]<<"\n";
					break;
			case 2: cout<<"Enter number of arguments for addition(1-10): ";
					cin>>buff.num;
					for(int i=0; i<buff.num; i++){
						cin>>buff.arg12[i];
					}

					status = send(sockid,&buff,sizeof(buff),0);
					if (status < 0){
						printf("\nClient Error: Writing to Server");
		   				return 0;
					}
					status = recv(sockid,&buff,sizeof(buff),0);
					if (status < 0){
						printf("Client Error: Reading from Server");
						return 0;
	   				}
	   				cout<<"Result is "<<buff.arg12[10]<<"\n";
					break;
			case 3: cout<<"Enter the dividend: ";
					cin>>buff.arg3[0];
					cout<<"Enter the divisor: ";
					cin>>buff.arg3[1];

					status = send(sockid,&buff,sizeof(buff),0);
					if (status < 0){
						printf("\nClient Error: Writing to Server");
		   				return 0;
					}
					status = recv(sockid,&buff,sizeof(buff),0);
					if (status < 0){
						printf("Client Error: Reading from Server");
						return 0;
	   				}
	   				cout<<"Quotient is "<<buff.arg3[2]<<" and remainder is "<<buff.arg3[3]<<"\n";
					break;

			case 4: status = send(sockid,&buff,sizeof(buff),0);
					if (status < 0){
						printf("\nClient Error: Writing to Server");
		   				return 0;
					}

		}
	}

    //cout<<buff.retval[0]<<"\n";
	close(status);
	close(sockid);
	printf("Program Closed Successfully\n");
	return 0;
}
