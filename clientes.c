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
#include <pthread.h>
#include <stdlib.h>
#include "definicoes.h"

#define N_T_CLIENTES 20

void* threadFunc() {

    int sockfd;
    char buffer[MAX_ENTRY_SIZE+1];
    requisicao_t req;

    int len;
    struct sockaddr_in address;
    int result;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = 9734;
    len = sizeof(address);


    while (1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tcp
        result = connect(sockfd, (struct sockaddr *)&address, len);
        if(result == -1) {
            printf("r: %d erro %d \n", result, errno);
            perror("error ao conectar ");
            exit(1);
        }

        int num = 1;
        req.escrever = num;
        req.posicao = (rand() % (N_CHUNKS * N_SERV_MEM)) * (TAM_MEM / N_CHUNKS);
        req.tam_buffer = (TAM_MEM / N_CHUNKS);
        memset(buffer, '\0', sizeof(buffer));
        for (int j=0; j<req.tam_buffer; j++) {
            buffer[j] = 65 + (rand() % 25);
        }

        write(sockfd, &req, sizeof(requisicao_t));
        write(sockfd, &buffer, req.tam_buffer * sizeof(char));

        //
        memset(buffer, '\0', sizeof(buffer));
        sleep(1);
        //
        req.escrever = 0;
        write(sockfd, &req, sizeof(requisicao_t));
        read(sockfd, &buffer, req.tam_buffer);
        printf("%s\n", buffer);

        close(sockfd);
    }

}

pthread_t threads[N_T_CLIENTES];
int main() {
    for (int i=0; i<N_T_CLIENTES; i++) {
        pthread_create(&threads[i], NULL, threadFunc, NULL);
    }
    for (int i=0; i<N_T_CLIENTES; i++) {
        pthread_join(threads[i], NULL);
    }

}
