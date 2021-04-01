#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "servidor.h"
#include <string.h>

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

    while (1) {
        Requisicao req;
        int num;
        printf("Voce quer escrever: (1/0) ");
        scanf("%d", &num);
        req.escrever = num;
        printf("Posicao: ");
        scanf("%d", &req.posicao);
        char buffer[100];
        if (req.escrever == 1) {
            printf("Mensagem: ");
            scanf("%s", buffer);
            req.tam_buffer = strlen(buffer);
        } else {
            printf("Tamanho da leitura: ");
            scanf("%d", &req.tam_buffer);
        }

        write(sockfd, &req, sizeof(Requisicao));

        if (req.escrever == 1) {
            write(sockfd, &buffer, sizeof(req.tam_buffer));
        } else {
            read(sockfd, &buffer, req.tam_buffer);
            printf("%s\0", buffer);
            printf("\n");
        }

    }
    // Requisicao req;
    // req.escrever =  0;
    // req.posicao = 8;
    // req.tam_buffer = 20;
    // // char buffer[] = "Ola Mundo Ola Mundo";
    // char buffer[20];
    //
    //
    //
    //
    // write(sockfd, &req, sizeof(Requisicao));
    // // write(sockfd, &buffer, sizeof(char) * req.tam_buffer);
    //
    //
    // read(sockfd, buffer, req.tam_buffer * sizeof(char));
    // printf("%s\0\n", buffer);
    // printf("\n");

    // int resposta, entrada;
    //
    // scanf("%d", &entrada);
    // write(sockfd, &entrada, sizeof(int));
    // read(sockfd, &resposta, sizeof(int));
    // printf("%d\n", resposta);

    close(sockfd);
    exit(0);
}
