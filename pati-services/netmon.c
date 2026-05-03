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

    FILE *log = fopen("/data/log/netmon.log", "a");
    if (!log) {
        fprintf(stderr, "[netmon] /data/log/netmon.log acilamadi\n");
        return 1;
    }

    fprintf(log, "[netmon] Daemon baslatildi (PID: %d)\n", getpid());
    fflush(log);

    int count = 0;
    while (running) {
        FILE *netdev = fopen("/proc/net/dev", "r");
        if (netdev) {
            char line[512];
            time_t now = time(NULL);
            char ts[64];
            strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

            fprintf(log, "[%s] === snapshot #%d ===\n", ts, ++count);

            fgets(line, sizeof(line), netdev);
            fgets(line, sizeof(line), netdev);

            while (fgets(line, sizeof(line), netdev)) {
                char iface[32];
                unsigned long rx_bytes, rx_packets, tx_bytes, tx_packets;
                if (sscanf(line, "%31[^:]: %lu %lu %*u %*u %*u %*u %*u %*u %lu %lu",
                           iface, &rx_bytes, &rx_packets, &tx_bytes, &tx_packets) == 4) {
                    fprintf(log, "  %-10s RX: %luB (%lu pkts)  TX: %luB (%lu pkts)\n",
                            iface, rx_bytes, rx_packets, tx_bytes, tx_packets);
                }
            }
            fclose(netdev);
            fflush(log);
        } else {
            fprintf(log, "[netmon] /proc/net/dev okunamadi\n");
        }

        sleep(15);
    }

    fprintf(log, "[netmon] Daemon kapandi\n");
    fclose(log);

    return 0;
}
