#define N_CLIENTES 250
#define N_SERV_MEM 2
#define SERVIDOR_REQ_N_THREADS 10

// #define TAM_MEM 1024 * 1024 // 1 MB
#define TAM_MEM 256
#define N_CHUNKS 8  // TAM_MEM tem q ser divisivel pelo N_CHUNKS

#define SERVIDOR_MEM_N_THREADS 30

#define FILENAME "logDados"
#define SLEEP_N_SEGUNDOS_REQ 20
#define SLEEP_N_SEGUNDOS_LOGGER 5

#define MAX_ENTRY_SIZE 200

typedef struct {
    char escrever;
    int posicao;
    int tam_buffer;
} requisicao_t;

typedef struct {
    int n_chunk;
    char dados[TAM_MEM / N_CHUNKS];
} chunkLogger_t;

#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
