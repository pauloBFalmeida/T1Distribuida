#define N_CLIENTES 250
#define N_SERV_MEM 1
#define SERVIDOR_REQ_N_THREADS 10

// #define TAM_MEM 1024 * 1024 // 1 MB
#define TAM_MEM 20

#define N_CHUNKS 5  // TAM_MEM tem q ser divisivel pelo N_CHUNKS
#define SERVIDOR_MEM_N_THREADS 30

#define FILENAME "logDados"
#define SLEEP_N_SEGUNDOS 20

typedef struct {
    char escrever;
    int posicao;
    int tam_buffer;
} Requisicao;

typedef struct {
    int n_chunk;
    char dados[TAM_MEM / N_CHUNKS];
} ChunkLogger;

#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
