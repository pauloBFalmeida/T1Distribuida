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
#include <sys/un.h>

char memoria[TAM_MEM];

pthread_mutex_t mutexes[N_CHUNKS+1];

pthread_t threadLogger;

int init() {
    for (int i=0; i<TAM_MEM; i++)   // loop nos caracteres de A-Z
        memoria[i] = 65 + (i % 25);
    for (int i=0; i<N_CHUNKS+1; i++)
        pthread_mutex_init(&mutexes[i], NULL);

//     pthread_t threads[N_THREADS];
//     pthread_create(&threads[i], NULL, compute_thread, &args[i]);
//
}

void *atenderLogger() {
    int server_sockfd;
    int client_sockfd;

    unsigned int server_len, client_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;

    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, "server_socket");
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, 255);

    return 0;
}


void *atenderCliente(void *arg) {
    int client_sockfd = *(int *)arg;

    printf("atendendo %d \n", client_sockfd);

    Requisicao req;
    read(client_sockfd, &req, sizeof(Requisicao));
    printf("Primeiro read recebido\n");
    // escrita
    char buffer[req.tam_buffer];
    if (req.escrever == 1) {
        read(client_sockfd, &buffer, (req.tam_buffer * sizeof(char)));
        printf("2 read recebido\n");
    }

    printf("esc: %s, pos: %d, tam: %d \n", &req.escrever, req.posicao, req.tam_buffer);
    printf("buffer: %s\n", buffer);

    int mutexInit = req.posicao / N_CHUNKS;
    int mutexFinal = (req.posicao + req.tam_buffer) / N_CHUNKS;

    // contador de posicoes do buffer
    int cont = 0;

    for (int i=mutexInit; i<=mutexFinal; i++) {
        int tam = min((i+1)*(TAM_MEM / N_CHUNKS),       // inicio do prox chunk
                        req.posicao + req.tam_buffer);  // posicao final do buffer na memoria
        int ini = max(i * (TAM_MEM / N_CHUNKS),         // inicio do chunk
                        req.posicao);                   // posicao inicial do buffer na memoria
        printf("tam: %d ini: %d mut: %d\n", tam, ini, i);
        pthread_mutex_lock(&mutexes[i]);
        printf("dentro do mutex\n");
        if (req.escrever == 1) {
            for (int j=ini; j<tam; j++) {
                memoria[j] = buffer[cont];
                printf("%c", buffer[cont]);
                cont++;
            }
        } else {    // read
            for (int j=ini; j<tam; j++) {
                buffer[cont] = memoria[j];
                printf("%c", buffer[cont]);
                cont++;
            }
        }
        printf("\n");
        pthread_mutex_unlock(&mutexes[i]);
    }

    printf("mem: ");
    for (int i=0; i<TAM_MEM; i++)
        printf("%c", memoria[i]);
    printf("\n");

    if (req.escrever != 1) {
        printf("bufferc '");
        for (int i=0; i<req.tam_buffer; i++)
            printf("%c", buffer[i]);
        printf("'\n");
        write(client_sockfd, &buffer, (req.tam_buffer * sizeof(char)) );
    }

    // write(client_sockfd, &resposta, sizeof(int));
}

int main() {
    init();
    // pthread_create(&threadLogger, NULL, atenderLogger, NULL);

    int server_sockfd;
    int client_sockfd;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    scanf("%hd", &server_address.sin_port);
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
    // serivdores de Requisicao
    close(client_sockfd);

    // fecha o servidor
    close(server_sockfd);
    exit(0);
}
