#define N_SERV_MEM 3
#define SERVIDOR_REQ_N_THREADS 10

// #define TAM_MEM 1024 * 1024 // 1 MB
#define TAM_MEM 10

#define N_CHUNKS 3
#define SERVIDOR_MEM_N_THREADS 30

typedef struct {
    char escrever;
    int posicao;
    int tam_buffer;
} Requisicao;

#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
