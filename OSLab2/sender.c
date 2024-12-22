#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	int sockfd;
	struct addrinfo hints;
	struct addrinfo *result;

	const char *host_name = "127.0.0.1";
	const char *port_number = "8845";

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((getaddrinfo(host_name, port_number, &hints, &result)) == -1) {
		perror("Failed to translate client socket");
		exit(EXIT_FAILURE);
	}

	if ((sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == -1) {
		printf("socket error\n");
		exit(EXIT_FAILURE);
	}

	if (connect(sockfd, result->ai_addr, result->ai_addrlen) == -1) {
		printf("connect error");
		exit(EXIT_FAILURE);
	}

	char *message = "Hello from client\n";
	send(sockfd, message, strlen(message), 0);
	printf("Message sent\n");

	close(sockfd);

	return 0;
}