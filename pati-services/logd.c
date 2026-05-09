#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

static volatile int running = 1;

void sigterm_handler(int sig) {
    (void)sig;
    running = 0;
}

int main(void) {
    signal(SIGTERM, sigterm_handler);

    mkdir("/data/log", 0755);

    FILE *log = fopen("/data/log/pati.log", "a");
    if (!log) {
        fprintf(stderr, "[logd] /data/log/pati.log acilamadi\n");
        return 1;
    }

    time_t now;
    char ts[64];

    fprintf(log, "[logd] Daemon baslatildi (PID: %d)\n", getpid());
    fflush(log);

    int count = 0;
    while (running) {
        now = time(NULL);
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

        fprintf(log, "[%s] logd heartbeat #%d\n", ts, ++count);
        fflush(log);

        sleep(10);
    }

    now = time(NULL);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(log, "[%s] logd kapandi, toplam %d heartbeat\n", ts, count);
    fclose(log);

    return 0;
}
