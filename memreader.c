#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig)
{
    stop = 1;
}

int main()
{
    signal(SIGINT, handle_sigint);

    while (!stop)
    {
        FILE *fptr = fopen("/proc/meminfo", "r");

        char mem_total[54], mem_free[54];

        fgets(mem_total, sizeof(mem_total), fptr);
        fgets(mem_free, sizeof(mem_free), fptr);

        mem_total[strcspn(mem_total, "\n")] = '\0';
        mem_free[strcspn(mem_free, "\n")] = '\0';

        printf("\r%s | %s", mem_total, mem_free);
        fflush(stdout);

        fclose(fptr);
        sleep(1);
    }

    printf("\nPrograma finalizado.\n");
    return 0;
}
