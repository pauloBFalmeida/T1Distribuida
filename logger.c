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
#include <errno.h>
#include "definicoes.h"

char memoriaLogger[TAM_MEM];
pthread_t threadCopia;
pthread_mutex_t mutex;


void copiaDaMemoria() {
    // criacao do socket local
    int sockfd_local;
    struct sockaddr_un address;
    int result;
    address.sun_family = AF_UNIX;   strcpy(address.sun_path, "local_socket");

    chunkLogger_t chunkLogger;

    sockfd_local = socket(AF_UNIX, SOCK_STREAM, 0);
    result = connect(sockfd_local, (struct sockaddr *)&address, sizeof(address));
    if(result == -1) {
        perror("erro ao conectar com server local");
        exit(1);
    }

    // pegar todos os chunks
    pthread_mutex_lock(&mutex);
    chunkLogger.n_chunk = 0;
    while(1) {
        read(sockfd_local, &chunkLogger, sizeof(chunkLogger_t));
        printf("%d: %s%c\n", chunkLogger.n_chunk, chunkLogger.dados, '\0');
        // fim dos chucks recebidos
        if (chunkLogger.n_chunk < 0) {
            break;
        } else {
            // recebe o chunk e salva ele na posicao da memoria
            int chunkStart = chunkLogger.n_chunk * (TAM_MEM / N_CHUNKS);
            for (int i= 0; i<(TAM_MEM / N_CHUNKS); i++) {
                memoriaLogger[chunkStart + i] = chunkLogger.dados[i];
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    close(sockfd_local);
}

void* manterAtualizada() {
    while(1) {
        copiaDaMemoria();
        sleep(SLEEP_N_SEGUNDOS_LOGGER);
    }
}

int main() {
    //
    pthread_mutex_init(&mutex, NULL);
    // criacao do socket de internet
    int server_sockfd, client_sockfd;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    // le o arquivo de config
    FILE *configFile;
    char addr[32+1];
    configFile = fopen("configLogger.txt", "r");
    fscanf(configFile,"%s", addr);
    server_address.sin_addr.s_addr = inet_addr(addr);
    fscanf(configFile,"%hd", &server_address.sin_port);
    fclose(configFile);
    printf("addr: %s port: %hd\n", addr, server_address.sin_port);

    server_len = sizeof(server_address);
    client_len = sizeof(client_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, 250);

    pthread_create(&threadCopia, NULL, manterAtualizada, NULL);
    //
    chunkLogger_t chunkLogger;
    while(1) {
        printf("logger esperando pelo serverReq\n");
        client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);

        // pega a memoria atulizada
        copiaDaMemoria();

        // enviar pro servidor
        pthread_mutex_lock(&mutex);
        for (int i=0; i<N_CHUNKS; i++) {
            chunkLogger.n_chunk = i;
            int chunkStart = i * (TAM_MEM / N_CHUNKS);
            for (int i= 0; i<(TAM_MEM / N_CHUNKS); i++) {
                chunkLogger.dados[i] = memoriaLogger[chunkStart + i];
            }
            // printf("envio: %s\n", chunkLogger.dados);
            write(client_sockfd, &chunkLogger, sizeof(chunkLogger_t));
        }
        pthread_mutex_unlock(&mutex);
        close(client_sockfd);
    }
    pthread_mutex_destroy(&mutex);
    close(server_sockfd);
    exit(0);
}
