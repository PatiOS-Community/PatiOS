#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/route.h>
#include <sys/stat.h>
#include <errno.h>
#include "pati-headers/pcg.h"

int main() {
    struct dirent *entry;
    pid_t pid;
    struct dirent **namelist;
    int n = scandir("/dev/pcgconfigs", &namelist, NULL, alphasort);
    if (n < 0) {
        perror("[UYARI]: pcgconfigs açılamadı, sistem devam edecek..");
        while(wait(NULL) > 0);
        return 0;
    }
    printf("----------------------------------------\n");
    printf("----- MAUVYD Konfigürasyon Sistemi -----\n");
    printf("-------------Dosya Sistemi--------------\n");
    printf("-------------  Açılıyor.. --------------\n");
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);
    mkdir("/data", 0755);
    mkdir("/data/paticommands", 0755);
    int ret = mount("/dev/vda", "/data", "ext4", 0, NULL);
    if (ret == 0) {
        printf("[TAMAM]: Kalıcı depolama aktifleştirildi.\n");

        DIR *d = opendir("/data/paticommands");
        if (d) {
            struct dirent *ent;
            while ((ent = readdir(d)) != NULL) {
                if (ent->d_name[0] == '.') continue;
                char kaynak[512], hedef[512];
                snprintf(kaynak, sizeof(kaynak), "/data/paticommands/%s", ent->d_name);
                snprintf(hedef, sizeof(hedef), "/lib/paticommands/%s", ent->d_name);
                
                struct stat st;
                if (stat(kaynak, &st) == 0 && S_ISREG(st.st_mode)) {
                    int in = open(kaynak, O_RDONLY);
                    int out = open(hedef, O_WRONLY | O_CREAT | O_TRUNC, 0755);
                    if (in >= 0 && out >= 0) {
                        char buf[4096];
                        ssize_t n;
                        while ((n = read(in, buf, sizeof(buf))) > 0)
                            write(out, buf, n);
                        printf("[TAMAM]: %s kopyalandi.\n", ent->d_name);
                    }
                    if (in >= 0) close(in);
                    if (out >= 0) close(out);
                }
            }
            closedir(d);
        } else {
            printf("[BILGI]: /data/paticommands bulunamadi, atlaniyor.\n");
        }
        putenv("PATH=/bin:/pcg-startup:/usr/bin:/lib/paticommands");
    } else if (errno == EBUSY) {
        printf("[BILGI]: Depolama zaten bagli.\n");
        putenv("PATH=/bin:/pcg-startup:/usr/bin:/lib/paticommands");
    } else {
        perror("[HATA]: Disk baglanamadi");
        putenv("PATH=/bin:/pcg-startup:/usr/bin:/lib/paticommands");
    }
    putenv("TERM=linux");
    printf("\nYaHnI oLaN vArMı?\n");
    printf("Pati-2.1 by Mehmet Demir. Kod adı: Ananas (Pineapple)\n");
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
    ifr.ifr_flags = IFF_UP | IFF_RUNNING;
    ioctl(fd, SIOCSIFFLAGS, &ifr);

    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, "10.0.2.15", &addr->sin_addr);
    ioctl(fd, SIOCSIFADDR, &ifr);

    inet_pton(AF_INET, "255.255.255.0", &addr->sin_addr);
    ioctl(fd, SIOCSIFNETMASK, &ifr);

    struct rtentry route;
    memset(&route, 0, sizeof(route));

    struct sockaddr_in *gw = (struct sockaddr_in *)&route.rt_gateway;
    gw->sin_family = AF_INET;
    inet_pton(AF_INET, "10.0.2.2", &gw->sin_addr);

    struct sockaddr_in *dst = (struct sockaddr_in *)&route.rt_dst;
    dst->sin_family = AF_INET;
    dst->sin_addr.s_addr = INADDR_ANY;

    struct sockaddr_in *mask = (struct sockaddr_in *)&route.rt_genmask;
    mask->sin_family = AF_INET;
    mask->sin_addr.s_addr = INADDR_ANY;

    route.rt_flags = RTF_UP | RTF_GATEWAY;
    route.rt_dev = "eth0";

    ioctl(fd, SIOCADDRT, &route);

    close(fd);

    FILE *resolv = fopen("/etc/resolv.conf", "w");
    if (resolv != NULL) {
        fprintf(resolv, "nameserver 8.8.8.8\n");
        fclose(resolv);
    }
for (int i = 0; i < n; i++) {
    entry = namelist[i];
    if (strcmp(entry->d_name, ".") == 0) {
        continue;
    };
    if (strcmp(entry->d_name, "..") == 0) {
        continue;
    };
      usleep(10000);
      printf("İşlem Buldum!: %s\n", entry->d_name);
      char tamyol[512];
      snprintf(tamyol, sizeof(tamyol), "/dev/pcgconfigs/%s", entry->d_name);
    char dosyayolu[256] = {0};
    char bekle_val[16] = {0};
    char izle_val[16] = {0};
    pcg_read(tamyol, "konumu", dosyayolu, sizeof(dosyayolu));
    if (dosyayolu[0] != '/' || strstr(dosyayolu, "..") != NULL) {
        printf("[GUVENLIK] Atlanıyor, geçersiz yol: %s\n", dosyayolu);
        free(namelist[i]);
        continue;
    }
    pcg_read(tamyol, "bekle", bekle_val, sizeof(bekle_val));
    pcg_read(tamyol, "izle", izle_val, sizeof(izle_val));
    char *args[] = {NULL, NULL};
    args[0] = dosyayolu;
    int bekle = (strcmp(bekle_val, "1") == 0);
    int izle = (strcmp(izle_val, "1") == 0);
        if (izle) printf("[INFO] %s işlemi Karabaş Tarafından izlenecektir.\n", dosyayolu);


      pid = fork(); // ÇATALLAMA ZAMANII!

    if (pid == -1) {
        perror("Çatalı Çatalladılar!1 (fork failed)");
        exit(EXIT_FAILURE);
        }
    if (pid > 0 && bekle == 1) {
        usleep(100000);
        wait(NULL);
        }

    if (pid == 0) {
        printf("Çocuk işlem şu servisi başlatıyor: %s\n", dosyayolu);
        execv(dosyayolu, args);
        perror("Oops, Pati hastalandı!");
        exit(EXIT_FAILURE);
        }
free(namelist[i]);
}
free(namelist);
while(wait(NULL) > 0);
}
