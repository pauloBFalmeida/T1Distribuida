#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "definicoes.h"

int sockfd;
char buffer[MAX_ENTRY_SIZE+1];
requisicao_t req;
// int escrita, posicao, tam_buffer;
// char bufferChar[100];
//
//
// void escrever(int posicao, char* buffer, int tam_buffer) {
//     req.escrever = 1;
//     req.posicao = posicao;
//     req.tam_buffer = tam_buffer;
//     write(sockfd, &req, sizeof(requisicao_t));
//     write(sockfd, buffer, tam_buffer * sizeof(char));
// }
//
// void ler(int posicao, int tam_buffer) {
//     req.escrever = 0;
//     req.posicao = posicao;
//     req.tam_buffer = tam_buffer;
//     write(sockfd, &req, sizeof(requisicao_t));
//     // le
//     memset(bufferChar, '\0', sizeof(bufferChar));
//     read(sockfd, &bufferChar, tam_buffer);
//     printf("%s%c\n", bufferChar, '\0');
// }



int main() {
    struct sockaddr_in address;
    int len = sizeof(address);
    int result;

    // ler arquivo de config
    FILE *configFile;
    configFile = fopen("configCliente.txt", "r");
    char addr[32+1];
    address.sin_family = AF_INET;
    fscanf(configFile,"%s", addr);
    address.sin_addr.s_addr = inet_addr(addr);
    fscanf(configFile,"%hd", &address.sin_port);
    printf("addr: %s port: %hd\n", addr, address.sin_port);
    fclose(configFile);

    int escrita;
    while (1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tcp
        result = connect(sockfd, (struct sockaddr *)&address, len);
        if(result == -1) {
            perror("error ao conectar");
            exit(1);
        }

        printf("Voce quer escrever: (1/0) ");
        scanf("%d", &escrita);
        req.escrever = escrita;
        printf("Posicao: ");
        scanf("%d", &req.posicao);
        printf("Tamanho do buffer seguido da Mensagem: ");
        scanf("%d ", &req.tam_buffer);
        if (escrita == 1) {
            fgets(buffer, MAX_ENTRY_SIZE, stdin);
            // scanf("%100[^\n]", buffer);
            printf("\n");
        }

        // escreve a requisicao no socket
        write(sockfd, &req, sizeof(requisicao_t));
        // se for escrita, escreve o buffer
        if (escrita == 1) {
            write(sockfd, &buffer, req.tam_buffer * sizeof(char));
        // se for leitura, limpa o buffer e escreve a resposta no buffer
        } else {
            memset(buffer, '\0', sizeof(buffer));
            read(sockfd, &buffer, req.tam_buffer);
            printf("%s\n", buffer);
        }

        close(sockfd);
    }
    exit(0);
}
