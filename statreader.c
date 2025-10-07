#include <stdio.h>
#include <string.h>
#include <unistd.h> // importa sleep

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

int main()
{
    CPU cpuInfos[MAX_CPUS];
    size_t cpuQuantities = 0;
    char line[MAX_LINE];
    char label[MAX_LABEL];

    while (1)
    {
        FILE *fptr = fopen("/proc/stat", "r");
        if (!fptr)
        {
            perror("Erro ao abrir /proc/stat");
            return 1;
        }

        cpuQuantities = 0;

        while (fgets(line, sizeof(line), fptr) != NULL)
        {
            if (cpuQuantities > MAX_CPUS)
            {
                print("ALERTA: Máximo de CPUs ultrapassado");
                break;
            }

            int n = sscanf(line, "%15s %15s %15s %15s %15s %15s %15s %15s %15s %15s %15s",
                           label,
                           cpuInfos[cpuQuantities].user,
                           cpuInfos[cpuQuantities].nice,
                           cpuInfos[cpuQuantities].system,
                           cpuInfos[cpuQuantities].idle,
                           cpuInfos[cpuQuantities].iowait,
                           cpuInfos[cpuQuantities].irq,
                           cpuInfos[cpuQuantities].softirq,
                           cpuInfos[cpuQuantities].steal,
                           cpuInfos[cpuQuantities].guest,
                           cpuInfos[cpuQuantities].guest_nice);

            // omite informações que não são diretamente do cpu (por enquanto)
            if (strncmp(label, "cpu", 3) != 0)
                continue;

            cpuQuantities++;
        }

        fclose(fptr);

        // Move o cursor para o topo da tela e limpa a tela
        // https://www.reddit.com/r/C_Programming/comments/1ez0vhl/whats_the_most_efficient_way_to_clear_the_terminal/?tl=pt-br
        printf("\033[H\033[J");

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

        sleep(1);
    }

    return 0;
}
