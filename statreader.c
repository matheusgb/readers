/* Define _POSIX_C_SOURCE para garantir disponibilidade das funções POSIX */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define MAX_ITEM 15
#define MAX_CPUS 50

#define MAX_LINE 256
#define MAX_LABEL 16

typedef struct
{                              // Gerencia:
    char user[MAX_ITEM];       // Programas comuns, execução de instruções normais de programas.
    char nice[MAX_ITEM];       // Processos de baixa prioridade.
    char system[MAX_ITEM];     // Kernel, grava algo em disco, aloca memória, cria threads, pacotes de rede.
    char idle[MAX_ITEM];       // Ociosidade.
    char iowait[MAX_ITEM];     // Ociosidade, mas diferente de idle, há trabalho pendente, CPU aguardando rede ou disco responder.
    char irq[MAX_ITEM];        // Interrupções de Hardware. (ex: placa de rede informando que recebeu um pacote, teclado digitando, SSD sinalizando que terminou uma escrita e etc...)
    char softirq[MAX_ITEM];    // Interrupções de Sofware. (ex: processamento de pacotes TCP, tarefas do kernel, etc...)
    char steal[MAX_ITEM];      // Competição entre VMs por CPU, se uma VM que compartilha hardware com outra for menos priorizada, steal sobe.
    char guest[MAX_ITEM];      // Tempo gasto executando máquinas virtuais no sistema.
    char guest_nice[MAX_ITEM]; // Análogo a nice mas no contexto de virtualização, VM configurada com prioridade reduzida apresenta aumento.
} CPU;

static int reporta_erro(char *mensagem);
static int chama_probe(void);

static char *pula_espacos(char *ptr, const char *end);
static char *copia_token(char *src, const char *end, char *dest, size_t max_len);

int main(int, char **)
{
    const struct timespec delay = {1, 0};
    struct timespec sobra = {
        0,
    };

    while (nanosleep(&delay, &sobra) == 0)
        (void)chama_probe();

    printf("\nPrograma finalizado.\n");

    if (sobra.tv_sec + sobra.tv_nsec > 0)
    {
        printf("nanosleep terminou com errno %d (%s), restando pausar por outros %ld.%09ld ns\n",
               errno, strerror(errno), sobra.tv_sec, sobra.tv_nsec);
    }

    return 0;
}

static int
reporta_erro(char *mensagem)
{
    fprintf(stderr, "%s (errno: %d, %s)\n", mensagem, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

static char *
pula_espacos(char *ptr, const char *end)
{
    while (ptr < end && (*ptr == ' ' || *ptr == '\t'))
        ptr++;
    return ptr;
}

static char *
copia_token(char *src, const char *end, char *dest, size_t max_len)
{
    size_t len = 0;

    src = pula_espacos(src, end);

    while (src < end && *src != ' ' && *src != '\t' && *src != '\n' && len < max_len - 1)
    {
        dest[len++] = *src++;
    }

    dest[len] = '\0';

    return src;
}

static int
chama_probe(void)
{
    CPU cpuInfos[MAX_CPUS];
    size_t cpuQuantities = 0;
    char buffer[4096];
    char *ptr;
    const char *end;
    char label[MAX_LABEL];
    struct timespec ts;
    ssize_t lidos;
    int fd;

    fd = open("/proc/stat", O_RDONLY);
    if (fd < 0)
        return reporta_erro("erro abrindo /proc/stat");

    clock_gettime(CLOCK_REALTIME, &ts);

    lidos = read(fd, buffer, sizeof(buffer));
    close(fd);

    if (lidos <= 0)
        return reporta_erro("erro lendo de /proc/stat");

    end = &buffer[lidos];

    ptr = buffer;
    while (ptr < end && cpuQuantities < MAX_CPUS)
    {
        char *linha_inicio = ptr;
        const char *linha_end;

        while (ptr < end && *ptr != '\n')
            ptr++;

        if (ptr >= end)
            break;

        linha_end = ptr;
        ptr++;

        char *pos = copia_token(linha_inicio, linha_end, label, MAX_LABEL);

        if (strncmp(label, "cpu", 3) != 0)
            continue;

        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].user, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].nice, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].system, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].idle, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].iowait, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].irq, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].softirq, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].steal, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].guest, MAX_ITEM);
        pos = copia_token(pos, linha_end, cpuInfos[cpuQuantities].guest_nice, MAX_ITEM);

        cpuQuantities++;
    }

    if (cpuQuantities >= MAX_CPUS)
    {
        fprintf(stderr, "ALERTA: Máximo de CPUs (%d) atingido ou ultrapassado\n", MAX_CPUS);
    }

    printf("Timestamp: %ld.%09ld\n", ts.tv_sec, ts.tv_nsec);

    for (size_t i = 0; i < cpuQuantities; i++)
    {
        printf("CPU %zu: user=%s, nice=%s, system=%s, idle=%s, iowait=%s, irq=%s, softirq=%s, steal=%s, guest=%s, guest_nice=%s\n",
               i,
               cpuInfos[i].user,
               cpuInfos[i].nice,
               cpuInfos[i].system,
               cpuInfos[i].idle,
               cpuInfos[i].iowait,
               cpuInfos[i].irq,
               cpuInfos[i].softirq,
               cpuInfos[i].steal,
               cpuInfos[i].guest,
               cpuInfos[i].guest_nice);
    }

    return 0;
}
