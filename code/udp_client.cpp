// Client side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define PORT	 8080 
#define MAXLINE 1024
#define IP_ADDRESS "insert here"

struct in_addr
GetInAddr(char *IPAddress)
{
	in_addr_t InAddrT = inet_addr(IPAddress);
	if(InAddrT == (in_addr_t)(-1))
	{
		perror("Unable to convert IP address as string to in_addr");
		exit(EXIT_FAILURE);
	}
	
	struct in_addr InAddr = {InAddrT};
	return InAddr;
}

int main() { 
	int sockfd; 
	char buffer[MAXLINE]; 
	const char *hello = "Hello from client"; 
	struct sockaddr_in	 servaddr; 

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(PORT);
	char *IPAddress = (char *)IP_ADDRESS;
	servaddr.sin_addr = GetInAddr(IPAddress); 
	
	int n;
	socklen_t len; 
	
	sendto(sockfd, (const char *)hello, strlen(hello), 
		MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
			sizeof(servaddr)); 
	std::cout<<"Hello message sent."<<std::endl; 
		
	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, (struct sockaddr *) &servaddr, 
				&len); 
	buffer[n] = '\0'; 
	std::cout<<"Server :"<<buffer<<std::endl; 

	close(sockfd); 
	return 0; 
}
