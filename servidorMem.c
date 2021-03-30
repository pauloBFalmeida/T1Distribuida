#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "servidor.h"

// TAM_MEM
char memoria[] = "123456789";

pthread_mutex_t mutexes[N_CHUNKS];

int init() {

    for (int i=0; i<N_CHUNKS; i++)
        pthread_mutex_init(&mutexes[i], NULL);

//     pthread_t threads[N_THREADS];
//     pthread_create(&threads[i], NULL, compute_thread, &args[i]);
//
}

// pthread_join(threads[i], (void**)&results[i]);
// pthread_create(&threads[i], NULL, compute_thread, &args[i]);

// pthread_mutex_t mutex;
// pthread_mutex_lock(&mutex);	// dou lock

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

    int mutexInit = req.posicao / N_CHUNKS;
    int mutexFinal = (req.posicao + req.tam_buffer) / N_CHUNKS;

    // contador de posicoes do buffer
    int cont = 0;

    for (int i=mutexInit; i<=mutexFinal; i++) {
        int tam = min((i+1)*(TAM_MEM / N_CHUNKS),       // inicio do prox chunk
                        req.posicao + req.tam_buffer);  // posicao final do buffer na memoria
        int ini = min(i * (TAM_MEM / N_CHUNKS),         // inicio do chunk
                        req.posicao);                   // posicao inicial do buffer na memoria
        pthread_mutex_lock(&mutexes[i]);
        printf("dentro do mutex\n");
        for (int j=ini; j<tam; j++) {
            memoria[j] = buffer[cont];
            cont++;
        }
        pthread_mutex_unlock(&mutexes[i]);
    }

    printf("mem: %s\n", memoria);

    // write(client_sockfd, &resposta, sizeof(int));
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
    close(server_sockfd);
    exit(0);
}
