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
#include <errno.h>
#include <semaphore.h>


int socketsServidoresMem[N_SERV_MEM];
struct sockaddr_in addressServidoresMem[N_SERV_MEM];
sem_t semaforosClientes;

void init() {
    for (int i=0; i<N_SERV_MEM; i++) {
        int sockfd;
        int len;
        int result;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tcp
        addressServidoresMem[i].sin_family = AF_INET;
        addressServidoresMem[i].sin_addr.s_addr = inet_addr("127.0.0.1");
        scanf("%hd", &addressServidoresMem[i].sin_port);
        socketsServidoresMem[i] = sockfd;
    }
}

void *atenderLogger(void *arg) {
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    scanf("%hd", &address.sin_port);
    len = sizeof(address);

    char recebido[TAM_MEM];
    char contador = 'A';
    FILE *file;
    while (1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tcp
        result = connect(sockfd, (struct sockaddr *)&address, len);
        if(result == -1) {
            printf("r: %d erro %d \n", result, errno);
            perror("error ao conectar ao logger");
            exit(1);
        }

        read(sockfd, &recebido, TAM_MEM * sizeof(char));
        close(sockfd);

        const char filename[] = {'l','o','g','D','a','d','o',contador,'.','t','x','t','\0'};
        contador++;

        file = fopen(filename, "w+");
        fputs(recebido, file);
        fclose(file);
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
    int serverFinal = (req.posicao + req.tam_buffer - 1) / TAM_MEM;
    int posBuffer = 0;
    printf("ServerInit: %d, serverFinal: %d \n", serverInit, serverFinal);

    char resposta[req.tam_buffer];
    int tam_buffer = req.tam_buffer;

    for (int i=serverInit; i<=serverFinal; i++) {
        errno = 0;
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tcp
        int r2 = connect(sockfd, (struct sockaddr *)&addressServidoresMem[i], sizeof(addressServidoresMem[i]));
        if (r2 < 0) {
            printf("r2: %d erro %d \n", r2, errno);
        }

        Requisicao req_i;
        req_i.escrever = req.escrever;
        req_i.posicao = max(req.posicao - i*TAM_MEM, 0);
        req_i.tam_buffer = min(min(TAM_MEM, TAM_MEM - req_i.posicao), req.tam_buffer);
        req.tam_buffer -= req_i.tam_buffer;
        // printf("tam_buffer: %d\n", req.tam_buffer);
        printf("Escrever no socket\n");
        int r = write(sockfd, &req_i, sizeof(Requisicao));
        if (r < 0) {
            printf("r: %d erro %d \n", r, errno);
        }
        printf("esc: %s, pos: %d, tam: %d \n", &req_i.escrever, req_i.posicao, req_i.tam_buffer);
        if (req.escrever == 1) {
            char sendBuff[req_i.tam_buffer];
            strncpy(sendBuff, &buffer[posBuffer], req_i.tam_buffer);
            write(sockfd, &sendBuff, (req_i.tam_buffer * sizeof(char)));
            printf("bufferc '");
            for (int i=0; i<req_i.tam_buffer; i++)
                printf("%c", sendBuff[i]);
            printf("'\n");
        } else {
            char recebido[tam_buffer];
            read(sockfd, &recebido, (req_i.tam_buffer * sizeof(char)));
            printf("recebido '");
            for (int i=0; i<req_i.tam_buffer; i++)
                printf("%c", recebido[i]);
            printf("'\n");
            strncpy(resposta + posBuffer, recebido, req_i.tam_buffer);
            printf("resposta '");
            for (int i=0; i<tam_buffer; i++)
                printf("%c", resposta[i]);
            printf("'\n");
        }
        posBuffer += req_i.tam_buffer;

        // close(socketsServidoresMem[i]);
        close(sockfd);
    }

    if (req.escrever != 1) {
        printf("resposta '");
        for (int i=0; i<tam_buffer; i++)
            printf("%c", resposta[i]);
        printf("'\n");
        write(client_sockfd, resposta, (tam_buffer * sizeof(char)) );
    }

    close(client_sockfd);
    sem_post(&semaforosClientes);
}

int main() {
    init();

    pthread_t threadLogger;
    pthread_create(&threadLogger, NULL, atenderLogger, NULL);

    int server_sockfd;
    int client_sockfd[N_CLIENTES];
    pthread_t client_thread[N_CLIENTES];
    sem_init(&semaforosClientes, 0, N_CLIENTES);
    int atual = 0;
    // int client_sockfd;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = 9734;
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, N_CLIENTES);

    while(1) {
        sem_wait(&semaforosClientes);
        printf("server waiting\n");
        client_len = sizeof(client_address);
        client_sockfd[atual] = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);

        pthread_create(&client_thread[atual], NULL, atenderCliente, (void*)&client_sockfd[atual]);

        atual = (atual + 1) % N_CLIENTES;
    }
    close(server_sockfd);

    sem_destroy(&semaforosClientes);

    exit(0);
}
