#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define PORT 8845
#define BACKLOG 1

volatile sig_atomic_t sighupReceived = 0;

void sighup_handler(int s) {
	sighupReceived = 1;
}

int main() {
	int sockfd, new_fd;
	int incomingSocketFD = 0;
	struct sockaddr_in socketAddress;
	int addressLength = sizeof(socketAddress);
	struct sigaction sa;
	sigset_t blockedMask, origMask;
	char buffer[4000] = {0};
	int readBytes;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("create error");
		exit(EXIT_FAILURE);
	}

	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = INADDR_ANY;
	socketAddress.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr*)& socketAddress, sizeof(socketAddress)) == -1) {
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	if (listen(sockfd, BACKLOG) < 0) {
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	printf("Server running on port: %d\n", PORT);

	sigaction(SIGHUP, NULL, &sa);
	sa.sa_handler = sighup_handler;
	sa.sa_flags |= SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);

	sigemptyset(&blockedMask);
	sigemptyset(&origMask);
	sigaddset(&blockedMask, SIGHUP);
	sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

	
	fd_set fds;
	int incoming_socket_fd = 0;
	int max_socket;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
	
		if (incoming_socket_fd > 0) {
			FD_SET(incoming_socket_fd, &fds);
		}

		max_socket = (incoming_socket_fd > sockfd) ? incoming_socket_fd : sockfd;

		if (pselect(max_socket + 1, &fds, NULL, NULL, NULL, &origMask) < 0 && errno != EINTR) {
			perror("Pselect error");
			exit(EXIT_FAILURE);
		}

		if (sighupReceived) {
			printf("SIGHUP received!");
			sighupReceived = 0;
			continue;
		}

		if (incoming_socket_fd > 0 && FD_ISSET(incoming_socket_fd, &fds)) {
			readBytes = read(incoming_socket_fd, buffer, 1024);

			if (readBytes > 0) {
				printf("Received data: %d bytes\n", readBytes);
			} else {
				if (readBytes == 0) {
					close(incoming_socket_fd);
					incoming_socket_fd = 0;
				} else {
					perror("read error");
				}

			}
			continue;
		}

		if (FD_ISSET(sockfd, &fds)) {
			if ((incoming_socket_fd = accept(sockfd, (struct sockaddr*)&socketAddress, (socklen_t*)&addressLength)) < 0) {
				perror("accept error");
				exit(EXIT_FAILURE);
			}

			printf("New connection \n");
		}
	}

	close(sockfd);

	return 0;
}