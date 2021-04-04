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
#include <errno.h>

int main() {

    int sockfd;
    struct sockaddr_un address;
    int result;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    address.sun_family = AF_UNIX;   strcpy(address.sun_path, "server_socket");
    result = connect(sockfd, (struct sockaddr *)&address, sizeof(address));
    if(result == -1) {
        printf("errno: %d\n", errno);
        perror("oops: client1");
        exit(1);
    }

    while(1) {
        ChunkLogger chunkLogger;
        chunkLogger.n_chunk = 0;
        read(sockfd, &chunkLogger, sizeof(ChunkLogger));
        printf("%d: %s%c\n", chunkLogger.n_chunk, chunkLogger.dados, '\0');
        if (chunkLogger.n_chunk < 0) {
            break;
        }
    }

    // 
    close(sockfd);

    exit(0);
}
