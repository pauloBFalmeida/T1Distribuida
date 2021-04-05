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
#include <semaphore.h>
#include "definicoes.h"

struct sockaddr_in serverReq_address;
struct sockaddr_in addressServidoresMem[N_SERV_MEM];
struct sockaddr_in addressLoggers[N_SERV_MEM];
int socket_cliente[N_CLIENTES];
int socketsLoggers[N_SERV_MEM];
sem_t semaforosClientes;

void init_address() {
    FILE *configFile;
    configFile = fopen("configServidorReq.txt", "r");
    char addr[32+1];
    // le o address desse server
    serverReq_address.sin_family = AF_INET;
    fscanf(configFile,"%s", addr);
    serverReq_address.sin_addr.s_addr = inet_addr(addr);
    fscanf(configFile,"%hd", &serverReq_address.sin_port);
    printf("addr: %s port: %hd\n", addr, serverReq_address.sin_port);
    // le o address dos servidores de Mem
    for (int i=0; i<N_SERV_MEM; i++) {
        addressServidoresMem[i].sin_family = AF_INET;
        fscanf(configFile,"%s", addr);
        serverReq_address.sin_addr.s_addr = inet_addr(addr);
        fscanf(configFile,"%hd", &addressServidoresMem[i].sin_port);
    }
    // le o address dos loggers
    for (int i=0; i<N_SERV_MEM; i++) {
        addressLoggers[i].sin_family = AF_INET;
        fscanf(configFile,"%s", addr);
        serverReq_address.sin_addr.s_addr = inet_addr(addr);
        fscanf(configFile,"%hd", &addressLoggers[i].sin_port);
    }
    fclose(configFile);
}

void *atenderLogger(void *arg) {
    int sockfd_logger;
    int len_address_logger = sizeof(addressLoggers[0]);
    int result_conexao;

    chunkLogger_t chunkLogger;
    char recebido[(TAM_MEM / N_CHUNKS)+1];
    recebido[TAM_MEM / N_CHUNKS] = '\0';
    int contador = 1;
    char scontador[5];
    char filename[] = "logDados01234.txt";
    FILE *file;
    while (1) {
        // int to char
        int aux = contador;
        for (int i=4; i>= 0; i--) {
            filename[8 + i] = 48 + (aux % 10);
            aux = aux / 10;
        }

        contador++;
        file = fopen(filename, "w+");
        // for todos os loggers
        for (int i=0; i<N_SERV_MEM; i++) {
            sockfd_logger = socket(AF_INET, SOCK_STREAM, 0);   // tcp
            result_conexao = connect(sockfd_logger, (struct sockaddr *)&addressLoggers[i], len_address_logger);
            if(result_conexao == -1) {
                printf("r: %d erro %d \n", result_conexao, errno);
                perror("error ao conectar ao logger");
                exit(1);
            }

            // conteudo recebido do logger em chunks
            printf("recebido: ");
            for (int i=0; i<N_CHUNKS; i++) {
                read(sockfd_logger, &chunkLogger, sizeof(chunkLogger_t));
                printf("%s", chunkLogger.dados);
                strncpy(recebido, chunkLogger.dados, (TAM_MEM / N_CHUNKS));
                fputs(recebido, file);
            }
            printf("\n");
            // fecha o socket desse logger
            close(sockfd_logger);
        }
        // acrescenta uma linha no final do arquivo e fecha ele
        fputs("\n", file);
        fclose(file);

        // espera SLEEP_N_SEGUNDOS para salvar o proximo log
        sleep(SLEEP_N_SEGUNDOS);
    }

}

void *atenderCliente(void *arg) {
    // pega o socket do cliente
    int socket_cliente = *(int *)arg;
    // le a requisicao do cliente
    requisicao_t req;
    read(socket_cliente, &req, sizeof(requisicao_t));
    char buffer[req.tam_buffer];
    // escrita
    if (req.escrever == 1) {
        read(socket_cliente, &buffer, (req.tam_buffer * sizeof(char)));
        // print do buffer
        for (int i=0; i<req.tam_buffer; i++)
            printf("%c", buffer[i]);
        printf("\n");
    }
    // calcula o server inicial e o final que a requisicao atinge
    int serverInit = req.posicao / TAM_MEM;
    int serverFinal = (req.posicao + req.tam_buffer - 1) / TAM_MEM;
    printf("ServerInit: %d, serverFinal: %d \n", serverInit, serverFinal);

    int posBuffer = 0;
    int tam_buffer = req.tam_buffer;
    char resposta[req.tam_buffer];
    int socket_servidorMem, result_serverMem;
    requisicao_t req_i;
    // se conecta com os servidores de memoria
    for (int i=serverInit; i<=serverFinal; i++) {
        socket_servidorMem = socket(AF_INET, SOCK_STREAM, 0);   // tcp
        result_serverMem = connect( socket_servidorMem,
                                    (struct sockaddr *)&addressServidoresMem[i],
                                    sizeof(addressServidoresMem[i])
                                    );
        if (result_serverMem < 0) {
            printf("result_serverMem: %d erro %d \n", result_serverMem, errno);
        }
        // cria a requisicao
        req_i.escrever = req.escrever;
        req_i.posicao = max(req.posicao - i*TAM_MEM, 0);
        req_i.tam_buffer = min(min(TAM_MEM, TAM_MEM - req_i.posicao), req.tam_buffer);
        req.tam_buffer -= req_i.tam_buffer;
        // escreve no socket do serverMem
        write(socket_servidorMem, &req_i, sizeof(requisicao_t));
        printf("esc: %c, pos: %d, tam: %d \n", (req_i.escrever +48), req_i.posicao, req_i.tam_buffer);
        // se for escrita, escrever o buffer
        if (req.escrever == 1) {
            char sendBuff[req_i.tam_buffer];
            strncpy(sendBuff, &buffer[posBuffer], req_i.tam_buffer);
            write(socket_servidorMem, &sendBuff, (req_i.tam_buffer * sizeof(char)));
            // print do conteudo do buffer
            printf("bufferc '");
            for (int i=0; i<req_i.tam_buffer; i++)
                printf("%c", sendBuff[i]);
            printf("'\n");
        // se for leitura
        } else {
            char recebido[tam_buffer];
            read(socket_servidorMem, &recebido, (req_i.tam_buffer * sizeof(char)));
            // print do conteudo do buffer
            printf("recebido '");
            for (int i=0; i<req_i.tam_buffer; i++)
                printf("%c", recebido[i]);
            printf("'\n");
            // resposta
            strncpy(resposta + posBuffer, recebido, req_i.tam_buffer);
            // print da resposta
            printf("resposta '");
            for (int i=0; i<tam_buffer; i++)
                printf("%c", resposta[i]);
            printf("'\n");
        }
        posBuffer += req_i.tam_buffer;
        // fecho o socket do servidorMem
        close(socket_servidorMem);
    }
    // se for leitura, envio a resposta pro cliente
    if (req.escrever != 1) {
        write(socket_cliente, resposta, (tam_buffer * sizeof(char)) );
        // print da resposta enviada
        printf("resposta '");
        for (int i=0; i<tam_buffer; i++)
            printf("%c", resposta[i]);
        printf("'\n");
    }
    // fecho o socket do cliente e libero um espaco pro prox thread cliente
    close(socket_cliente);
    sem_post(&semaforosClientes);
}

int main() {
    // leitura dos adress
    init_address();
    // thread logger
    pthread_t threadLogger;
    pthread_create(&threadLogger, NULL, atenderLogger, NULL);
    // thread para cada cliente
    pthread_t client_thread[N_CLIENTES];
    sem_init(&semaforosClientes, 0, N_CLIENTES);
    int atual = 0;
    // criacao do socket do servidor
    unsigned int server_len;
    struct sockaddr_in client_address;
    int client_len = sizeof(client_address);
    int sockfd_serverReq = socket(AF_INET, SOCK_STREAM, 0);
    server_len = sizeof(serverReq_address);
    bind(sockfd_serverReq, (struct sockaddr *)&serverReq_address, server_len);
    listen(sockfd_serverReq, N_CLIENTES);
    //
    while(1) {
        sem_wait(&semaforosClientes);
        printf("server esperando cliente\n");
        // cria uma thread pra cuidar do cliente
        socket_cliente[atual] = accept(sockfd_serverReq,(struct sockaddr *)&client_address, &client_len);
        pthread_create(&client_thread[atual], NULL, atenderCliente, (void*)&socket_cliente[atual]);
        atual = (atual + 1) % N_CLIENTES;
    }
    // fecha o socket e destroi os semaforos
    close(sockfd_serverReq);
    sem_destroy(&semaforosClientes);
    exit(0);
}
