#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define TAM_MEM 1024 * 1024 // 1 MB
#define N_CHUNKS 64
#define N_THREADS 30


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
    read(client_sockfd, &msg, sizeMsg);
    //

    //
    close(client_sockfd);
}

int main() {
    int server_sockfd, client_sockfd;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    // server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // server_address.sin_family = AF_INET;
    // server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    // server_address.sin_port = 9734;
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, "server_socket");
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, 255);

    while(1) {
        printf("server waiting\n");
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
        pthread_create(NULL, NULL, atenderCliente, (void*)&client_sockfd));

        // while(1) {
        //     read(client_sockfd, &msg, sizeMsg);
        //
        //     close(client_sockfd);
        // }
    }
    close(server_sockfd);
    exit(0);
}
