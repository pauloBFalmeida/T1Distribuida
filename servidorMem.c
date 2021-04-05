#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/un.h>
#include <semaphore.h>
#include <errno.h>
#include "definicoes.h"

char memoria[TAM_MEM];
pthread_mutex_t mutexes[N_CHUNKS];

pthread_t threadLogger;
char chunks_modificados[N_CHUNKS];

pthread_t client_thread[N_CLIENTES];
sem_t semaforosClientes;

int init() {
    for (int i=0; i<TAM_MEM; i++)   // loop nos caracteres de A-Z
        memoria[i] = 42;
        // memoria[i] = 65 + (i % 25);
    for (int i=0; i<N_CHUNKS; i++) {
        pthread_mutex_init(&mutexes[i], NULL);
        chunks_modificados[i] = 1;
    }

//     pthread_t threads[N_THREADS];
//     pthread_create(&threads[i], NULL, compute_thread, &args[i]);
//
}

void *atenderLogger() {
    int server_sockfd;
    int client_sockfd;

    unsigned int server_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;

    // deleta socket antigo de outra execucao
    remove("local_socket");

    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, "local_socket");
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, 250);

    unsigned int len = sizeof(client_address);
    while(1) {
        printf("server waiting logger\n");
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &len);
        printf("aceito client_sockfd: %d\n", client_sockfd);
        printf("errno: %d\n", errno);


        chunkLogger_t chunkLogger;
        for (int i=0; i<N_CHUNKS; i++) {
            char mudou = 0;
            chunkLogger.n_chunk = i;
            pthread_mutex_lock(&mutexes[i]);
            if (chunks_modificados[i]) {
                chunks_modificados[i] = 0;
                mudou = 1;
                for (int j=0; j<TAM_MEM / N_CHUNKS; j++) {
                    chunkLogger.dados[j] = memoria[j + (TAM_MEM / N_CHUNKS) * i];
                    printf("%c", chunkLogger.dados[j]);
                }
                printf("\n");
            }
            pthread_mutex_unlock(&mutexes[i]);

            if (mudou) {
                write(client_sockfd, &chunkLogger, sizeof(chunkLogger_t));
            }
        }
        //
        chunkLogger.n_chunk = -1;
        write(client_sockfd, &chunkLogger, sizeof(chunkLogger_t));

        close(client_sockfd);
        printf("cliente fechado\n");
    }

    close(server_sockfd);
}


void *atenderCliente(void *arg) {
    int client_sockfd = *(int *)arg;

    printf("atendendo %d \n", client_sockfd);

    requisicao_t req;
    read(client_sockfd, &req, sizeof(requisicao_t));
    // escrita
    char buffer[req.tam_buffer];
    if (req.escrever == 1) {
        read(client_sockfd, &buffer, (req.tam_buffer * sizeof(char)));
    }

    printf("esc: %c, pos: %d, tam: %d \n", req.escrever +48, req.posicao, req.tam_buffer);
    printf("buffer: %s\n", buffer);

    int mutexInit = req.posicao / (TAM_MEM / N_CHUNKS);
    int mutexFinal = (req.posicao + req.tam_buffer -1) / (TAM_MEM / N_CHUNKS);

    printf("mutexInit: %d mutexFinal: %d\n", mutexInit, mutexFinal);
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
            chunks_modificados[i] = 1;
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

    close(client_sockfd);
    sem_post(&semaforosClientes);
}

int main() {
    init();
    pthread_create(&threadLogger, NULL, atenderLogger, NULL);

    int server_sockfd;
    int atual = 0;
    int client_sockfd[N_CLIENTES];
    sem_init(&semaforosClientes, 0, N_CLIENTES);

    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;

    FILE *configFile;
    char addr[32+1];
    configFile = fopen("configServidorMem.txt", "r");
    fscanf(configFile,"%s", addr);
    server_address.sin_addr.s_addr = inet_addr(addr);
    fscanf(configFile,"%hd", &server_address.sin_port);
    fclose(configFile);
    printf("addr: %s port: %hd\n", addr, server_address.sin_port);

    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, N_CLIENTES);
    client_len = sizeof(client_address);

    while(1) {
        sem_wait(&semaforosClientes);
        printf("server waiting\n");
        client_sockfd[atual] = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);

        pthread_create(&client_thread[atual], NULL, atenderCliente, (void*)&client_sockfd[atual]);

        atual = (atual + 1) % N_CLIENTES;
    }

    // fecha o servidor
    close(server_sockfd);

    sem_destroy(&semaforosClientes);
    for (int i=0; i<N_CHUNKS; i++) {
        pthread_mutex_destroy(&mutexes[i]);
    }

    exit(0);
}
