#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int protocol;
int port = 0;
int sockfd;
struct sockaddr_in serverAddress;
struct sockaddr_in clientAddress;

void read_cmdline(int argc, char *argv[]) {
    int opt = 0;
    while ((opt = getopt(argc, argv, "t:p:")) != -1) {
        if (opt=='t') {
            if (!strcmp(optarg,"tcp")) {
                protocol = 1;
            }
            else if (!strcmp(optarg,"udp")) {
                protocol = 2;
            }
        }
        else if (opt=='p') {
            port = atoi(optarg);
        }
        else if (opt== '?') {
            fprintf(stderr, "Option -%c error\n", optopt);
            exit(1);
        }
        else {
            fprintf(stderr, "Error\n");
            exit(1);
        }
    }
}

void acceptTCP() {
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_addr.s_addr=INADDR_ANY;
    serverAddress.sin_port=htons(port);
	// create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1) {
        fprintf(stderr, "Could not open socket\n");
        exit(1);
    }
    if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Bind failed\n");
        exit(1);
    }
    int lis = listen(sockfd, 1);
    if (lis == -1) {
        fprintf(stderr, "Listen failed\n");
    }
    socklen_t addrsize = sizeof clientAddress;

    // accept client connection
    int new_fd = accept(sockfd, (struct sockaddr *)&clientAddress, &addrsize);

    if (new_fd < 0) {
        fprintf(stderr, "Accept failed\n");
        exit(1);
    }
    // receive from client
    long msg;
    char *data = (char *)&msg;
    int size = sizeof(msg);
    int bytes = recv(new_fd, data, size, 0);
    if (bytes <= 0) {
        perror("Error reading client socket");
        exit(1);
    }
    int num = ntohl(msg);
    printf("The number sent is: %d\n", num);
	int transmit = 1;
    uint8_t netmsg = htonl(transmit);
    // Send byte back to client
    if ((bytes = send(new_fd, &netmsg, sizeof netmsg, 0)) < 0) {
        perror("Error sending to client\n");
        exit(1);
    }
	close(sockfd);
    close(new_fd);
}

void acceptUDP() {
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_addr.s_addr=INADDR_ANY;
    serverAddress.sin_port=htons(port);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "Could not open socket\n");
        exit(1);
    }
    memset(serverAddress.sin_zero, '\0', sizeof(serverAddress.sin_zero));
    memset(&clientAddress, 0, sizeof(clientAddress));

    int bytes = bind(sockfd, (struct sockaddr *)&serverAddress, sizeof serverAddress);
    if (bytes < 0) {
        fprintf(stderr, "Bind failed!\n");
        exit(1);
    }

    socklen_t length = sizeof(clientAddress);
    long msg;
	char *buff = (char *)&msg;
    int n = recvfrom(sockfd, buff, sizeof buff, 0, (struct sockaddr *) &clientAddress, &length);
    if ( n < 0) {
        printf("Receiving failed\n");
        exit(1);
    }

    int num = ntohl(msg);
    printf("The number sent is: %d\n", num);
	int transmit = 1;
	uint8_t netbyte = htonl(transmit);
	socklen_t dest = sizeof(clientAddress);
    ssize_t byte = sendto(sockfd, &netbyte, sizeof netbyte, 0,
                            (struct sockaddr *)&clientAddress, dest);
	if (byte < 0) {
        fprintf(stderr, "sendto()\n");
        exit(1);
    }
    close(sockfd);
}

int main(int argc, char *argv[]) {
    // verify arg count
    if (argc != 5) {
        printf("Usage: %s -t <udp or tcp> -p <portNum>\n", argv[0]);
        exit(1);
    }
    read_cmdline(argc, argv);
    // connect and receive from client
    if (port && protocol) {
        if (protocol==1) {
            acceptTCP();
        }
        else if (protocol==2) {
            acceptUDP();
        }
    }
    else {
        printf("Usage: %s -x <data> -t <udp or tcp> -s <ip> -p <port>\n", argv[0]);
    }
    return 0;
}
