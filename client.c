#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXDATASIZE 100

int msg=0;
int protocol;
char *ip;
int port=0;
int sock;
struct sockaddr_in serverAddress;

void read_cmdline(int argc, char *argv[]) {
    int opt = 0;
    while ((opt = getopt(argc, argv, "t:p:s:x:")) != -1) {
        if (opt=='x') {
            msg = atoi(optarg);
        }
        else if (opt=='t') {
            if (!strcmp(optarg,"tcp")) {
                protocol = 1;
            }
            else if (!strcmp(optarg,"udp")) {
                protocol = 2;
            }
        }
        else if (opt=='s') {
            ip = optarg;
        }
        else if (opt=='p') {
            port = atoi(optarg);
        }
        else if (opt=='?') {
            fprintf(stderr, "Option -%c error\n", optopt);
            exit(1);
        }
        else {
            fprintf(stderr, "Error\n");
            exit(1);
        }
    }
}

void sendTCP() {
    // make socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    // connect socket
    if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress))<0) {
        perror("Connection failed\n");
        exit(1);
    }
    // convert to network byte order
    uint32_t netbyte = htonl(msg);
    // send data
    ssize_t bytes = send(sock, &netbyte, sizeof netbyte, 0);
    if (bytes < 0) {
        perror("Error sending to server\n");
        exit(1);
    }
    printf("Sent %d to server %s:%d via TCP\n", msg, ip, port);
    // receive from server
	long rec;
    char *buff = (char *)&rec;
    bytes = recv(sock, buff, MAXDATASIZE, 0);
	if (bytes) {
		printf("Success!\n");
	}
    printf("Bytes received: %ld\n", bytes);

    close(sock);
}

void sendUDP() {
    struct sockaddr addr;
    char str[INET_ADDRSTRLEN];
    char buff[MAXDATASIZE];
    // make and bind socket
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(serverAddress.sin_zero, '\0', sizeof(serverAddress.sin_zero));
    bind(sock, (struct sockaddr*)&serverAddress, sizeof serverAddress);
    // convert to network byte order
    uint32_t netbyte = htonl(msg);
    // send to server
    socklen_t dest = sizeof(serverAddress);
    ssize_t bytes = sendto(sock, &netbyte, sizeof netbyte, 0,
                            (struct sockaddr *)&serverAddress, dest);
    if (bytes < 0) {
        fprintf(stderr, "sendto()\n");
        exit(1);
    }
    printf("Sent %d to server %s:%d via UDP\n", msg, ip, port);
    
    socklen_t fromlen = sizeof addr;
    int byte_count = recvfrom(sock, buff, sizeof buff, 0, &addr, &fromlen);
	if (byte_count) {
		printf("Success!\n");
	}

    printf("recv()'d %d bytes of data in buf\n", byte_count);
    printf("from IP address %s\n",
            inet_ntop(AF_INET, (&((struct sockaddr_in *)&addr)->sin_addr), str,
            INET_ADDRSTRLEN));
    close(sock);
}

int main(int argc, char *argv[]) {
    // verify arg count
    if (argc != 9) {
        printf("Usage: %s -x <data> -t <udp or tcp> -s <ip> -p <port>\n", argv[0]);
        exit(1);
    }
    read_cmdline(argc, argv);
    // connect and send to server
    if (port && msg && protocol) {
        if (protocol==1) {
            sendTCP();
        }
        else if (protocol==2) {
            sendUDP();
        }
    }
    // not accepted or missing options
    else {
        printf("Usage: %s -x <data> -t <udp or tcp> -s <ip> -p <port>\n", argv[0]);
    }
    return 0;
}

