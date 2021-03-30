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


int socketsServidoresMem[N_SERV_MEM];

void init() {
    for (int i=0; i<N_SERV_MEM; i++) {
        int sockfd;
        int len;
        struct sockaddr_in address;
        int result;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tcp
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr("127.0.0.1");
        scanf("%hd", &address.sin_port);
        socketsServidoresMem[i] = sockfd;
        len = sizeof(address);

        result = connect(sockfd, (struct sockaddr *)&address, len);
        if(result == -1) {
            perror("error ao conectar ");
            exit(1);
        }
    }

}

void *atenderCliente(void *arg) {
    int client_sockfd = *(int *)arg;

    // escreve(posicao, &buffer, tamBuffer)
    Requisicao req;
    read(client_sockfd, &req, sizeof(Requisicao));
    // escrita
    char buffer[req.tam_buffer];
    if (req.escrever == 1) {
        read(client_sockfd, &buffer, (req.tam_buffer * sizeof(char)));
        for (int i=0; i<req.tam_buffer; i++)
            printf("%c", buffer[i]);
        printf("\n");
    }

    int serverInit = req.posicao / TAM_MEM;
    int serverFinal = (req.posicao + req.tam_buffer) / TAM_MEM;
    int posBuffer = 0;
    printf("ServerInit: %d, serverFinal: %d \n", serverInit, serverFinal);

    for (int i=serverInit; i<=serverFinal; i++) {
        Requisicao req_i;
        req_i.escrever = req.escrever;
        req_i.posicao = max(req.posicao - i*TAM_MEM, 0);
        req_i.tam_buffer = min(min(TAM_MEM, TAM_MEM - req_i.posicao), req.tam_buffer);
        req.tam_buffer -= req_i.tam_buffer;
        // printf("tam_buffer: %d\n", req.tam_buffer);
        write(socketsServidoresMem[i], &req_i, sizeof(Requisicao));
        printf("esc: %s, pos: %d, tam: %d \n", &req_i.escrever, req_i.posicao, req_i.tam_buffer);
        if (req.escrever == 1) {
            char sendBuff[req_i.tam_buffer];
            strncpy(sendBuff, &buffer[posBuffer], req_i.tam_buffer);
            write(socketsServidoresMem[i], &sendBuff, (req_i.tam_buffer * sizeof(char)));
            printf("bufferc '");
            for (int i=0; i<req_i.tam_buffer; i++)
                printf("%c", sendBuff[i]);
            printf("'\n");
            posBuffer += req_i.tam_buffer;
        }
    }

    close(client_sockfd);
}

int main() {
    init();
    int server_sockfd;
    int client_sockfd;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = 9734;
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, 255);

    while(1) {
        printf("server waiting\n");
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);

        pthread_t thread;
        pthread_create(&thread, NULL, atenderCliente, (void*)&client_sockfd);
    }
    close(server_sockfd);
    exit(0);
}
