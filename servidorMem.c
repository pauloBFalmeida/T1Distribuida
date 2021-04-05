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
    for (int i=0; i<TAM_MEM; i++)   // inicia memoria com '*'
        memoria[i] = 42;
    for (int i=0; i<N_CHUNKS; i++) {
        pthread_mutex_init(&mutexes[i], NULL);
        chunks_modificados[i] = 1;
    }
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

    chunkLogger_t chunkLogger;
    while(1) {
        printf("serverMem esperando logger\n");
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &len);
        if (client_sockfd < 0) {
            printf("errno: %d\n", errno);
        }

        // copiando os chunks de memoria que foram alterados pro logger
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
        // envia um chunkLogger com -1 pra finalizar
        chunkLogger.n_chunk = -1;
        write(client_sockfd, &chunkLogger, sizeof(chunkLogger_t));
        // fecha o socket
        close(client_sockfd);
    }
    //
    close(server_sockfd);
}


void *atenderCliente(void *arg) {
    // recebe o socket do cliente
    int client_sockfd = *(int *)arg;
    // cria a requisicao
    requisicao_t req;
    read(client_sockfd, &req, sizeof(requisicao_t));
    char buffer[req.tam_buffer];
    // caso escrita, ler o buffer de dados
    if (req.escrever == 1) {
        read(client_sockfd, &buffer, (req.tam_buffer * sizeof(char)));
    }

    printf("esc: %c, pos: %d, tam: %d \n", req.escrever +48, req.posicao, req.tam_buffer);
    printf("buffer: %s\n", buffer);

    // calcula os mutex que estao na requisicao
    int mutexInit = req.posicao / (TAM_MEM / N_CHUNKS);
    int mutexFinal = (req.posicao + req.tam_buffer -1) / (TAM_MEM / N_CHUNKS);
    // printf("mutexInit: %d mutexFinal: %d\n", mutexInit, mutexFinal);

    // contador de posicoes do buffer
    int cont = 0;
    // percorre todos os mutex
    for (int i=mutexInit; i<=mutexFinal; i++) {
        int tam = min((i+1)*(TAM_MEM / N_CHUNKS),       // inicio do prox chunk
                        req.posicao + req.tam_buffer);  // posicao final do buffer na memoria
        int ini = max(i * (TAM_MEM / N_CHUNKS),         // inicio do chunk
                        req.posicao);                   // posicao inicial do buffer na memoria
        printf("tam: %d ini: %d mut: %d\n", tam, ini, i);
        pthread_mutex_lock(&mutexes[i]);
        printf("dentro do mutex %d\n", i);
        // escreve o conteudo do buffer na posicao
        if (req.escrever == 1) {
            chunks_modificados[i] = 1;
            for (int j=ini; j<tam; j++) {
                memoria[j] = buffer[cont];
                printf("%c", buffer[cont]);
                cont++;
            }
        // leitura
        } else {
            for (int j=ini; j<tam; j++) {
                buffer[cont] = memoria[j];
                printf("%c", buffer[cont]);
                cont++;
            }
        }
        printf("\n");
        pthread_mutex_unlock(&mutexes[i]);
    }
    // print conteudo da memoria
    printf("mem: ");
    for (int i=0; i<TAM_MEM; i++)
        printf("%c", memoria[i]);
    printf("\n");
    // se for escrita, escrevemos o conteudo no socket
    if (req.escrever != 1) {
        write(client_sockfd, &buffer, (req.tam_buffer * sizeof(char)) );
        // print do buffer
        printf("bufferc '");
        for (int i=0; i<req.tam_buffer; i++)
            printf("%c", buffer[i]);
        printf("'\n");
    }
    // fecha o socket e libera um semaforo pra outra thread de cliente
    close(client_sockfd);
    sem_post(&semaforosClientes);
}

int main() {
    init();
    pthread_create(&threadLogger, NULL, atenderLogger, NULL);

    //
    int server_sockfd;
    int client_sockfd[N_CLIENTES];
    sem_init(&semaforosClientes, 0, N_CLIENTES);
    int atual = 0;
    // criacao do socket
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    // le o arquivo de config
    FILE *configFile;
    char addr[32+1];
    configFile = fopen("configServidorMem.txt", "r");
    fscanf(configFile,"%s", addr);
    server_address.sin_addr.s_addr = inet_addr(addr);
    fscanf(configFile,"%hd", &server_address.sin_port);
    fclose(configFile);
    printf("addr: %s port: %hd\n", addr, server_address.sin_port);
    //
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, N_CLIENTES);
    client_len = sizeof(client_address);

    while(1) {
        sem_wait(&semaforosClientes);
        printf("servidorMem esperando cliente\n");

        // cria uma thread pro cliente
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
