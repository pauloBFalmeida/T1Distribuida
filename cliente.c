#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

int main() {
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tcp
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = 9734;
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr *)&address, len);
    if(result == -1) {
        perror("error ao conectar ");
        exit(1);
    }

    int resposta, entrada;

    // while(1) {
        scanf("%d", &entrada);
        write(sockfd, &entrada, sizeof(int));
        read(sockfd, &resposta, sizeof(int));
        printf("%d\n", resposta);
    // }

    close(sockfd);
    exit(0);
}
