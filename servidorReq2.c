#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h>
#include "servidor.h"


typedef struct {
    struct sockaddr_in address;
} ServidorMem;

ServidorMem servidoresMem[N_SERV_MEM];

// void init() {
//     int entrada;
//     for (int i=0; i<N_SERV_MEM; i++) {
//         scanf("%d", entrada);
//         servidoresMem[i].address.sun_path = entrada;
//     }
//
//     result = connect(sockfd, (struct sockaddr *)&address, len);
// }

void *atenderCliente(void *arg) {
    int client_sockfd = *(int *)arg;

    int resposta;
    read(client_sockfd, &resposta, sizeof(int));
    resposta++;
    write(client_sockfd, &resposta, sizeof(int));
    close(client_sockfd);
}

int main() {
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

        // pthread_t thread;
        // int r = pthread_create(&thread, NULL, atenderCliente, (void*)&client_sockfd);
        #pragma omp parallel num_threads(SERVIDOR_REQ_N_THREADS)
        {
            #pragma omp task
            atenderCliente((void*)&client_sockfd);
        }
    }
    close(server_sockfd);
    exit(0);
}
