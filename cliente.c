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

void escrever(int posicao, int tam_buffer) {
    req.escrever = 1;
    req.posicao = posicao;
    req.tam_buffer = tam_buffer;
    write(sockfd, &req, sizeof(requisicao_t));
    write(sockfd, buffer, tam_buffer * sizeof(char));
}

void ler(int posicao, int tam_buffer) {
    req.escrever = 0;
    req.posicao = posicao;
    req.tam_buffer = tam_buffer;
    write(sockfd, &req, sizeof(requisicao_t));
    // limpa o buffer e le o conteudo do socket
    memset(buffer, '\0', sizeof(buffer));
    read(sockfd, &buffer, tam_buffer);
    printf("%s%c\n", buffer, '\0');
}

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

    int escrita, posicao, tam_buffer;
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
        scanf("%d", &posicao);
        printf("Tamanho do buffer e a mensagem(caso leitura entre 0): ");
        scanf("%d ", &tam_buffer);
        if (escrita == 1) {
            fgets(buffer, MAX_ENTRY_SIZE, stdin);
            // scanf("%"MAX_ENTRY_SIZE"[^\n]", buffer);
            escrever(posicao, tam_buffer);
        } else {
            ler(posicao, tam_buffer);
        }


        close(sockfd);
    }
    exit(0);
}
