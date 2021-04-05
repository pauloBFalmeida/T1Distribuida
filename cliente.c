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
    int len;
    struct sockaddr_in address;
    int result;


    FILE *configFile;
    configFile = fopen("configCliente.txt", "r");
    char addr[32+1];
    // le o address desse server
    address.sin_family = AF_INET;
    fscanf(configFile,"%s", addr);
    address.sin_addr.s_addr = inet_addr(addr);
    fscanf(configFile,"%hd", &address.sin_port);
    printf("addr: %s port: %hd\n", addr, address.sin_port);
    fclose(configFile);

    len = sizeof(address);


    while (1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tcp
        result = connect(sockfd, (struct sockaddr *)&address, len);
        if(result == -1) {
            printf("r: %d erro %d \n", result, errno);
            perror("error ao conectar ");
            exit(1);
        }

        int num;
        printf("Voce quer escrever: (1/0) ");
        scanf("%d", &num);
        req.escrever = num;
        printf("Posicao: ");
        scanf("%d", &req.posicao);
        printf("Tamanho do buffer seguido da Mensagem: ");
        scanf("%d ", &req.tam_buffer);
        if (req.escrever == 1) {
            fgets(buffer, MAX_ENTRY_SIZE, stdin);
            // scanf("%100[^\n]", buffer);
            printf("\n");
        }

        write(sockfd, &req, sizeof(requisicao_t));

        if (req.escrever == 1) {
            write(sockfd, &buffer, req.tam_buffer * sizeof(char));
        } else {
            memset(buffer, '\0', sizeof(buffer));
            read(sockfd, &buffer, req.tam_buffer);
            printf("%s\n", buffer);
        }


        // printf("Voce quer escrever: (1/0) ");
        // scanf("%d", &escrita);
        //
        // printf("Posicao: ");
        // scanf("%d", &posicao);
        // printf("Tamanho do buffer: ");
        // scanf("%d", &tam_buffer);
        // if (escrita == 1) {
        //     printf("Mensagem: ");
        //     scanf("%s", bufferChar);
        //     escrever(posicao, bufferChar, tam_buffer);
        // } else {
        //     ler(posicao, tam_buffer);
        // }

        close(sockfd);
    }

    exit(0);
}
