#include "pcg.h"

int pcg_read(const char *file_dst, const char *key, char *buf, size_t buf_sz) {
    FILE *f = fopen(file_dst, "r");
    if (!f) return -1;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *k = strtok(line, " =\n");
        if (!k) continue;
        if (strcmp(k, key) == 0) {
            char *v = strtok(NULL, " =\n");
            if (v) {
                strncpy(buf, v, buf_sz - 1);
                buf[buf_sz - 1] = '\0';
                fclose(f);
                return 0;
            }
        }
    }
    fclose(f);
    return -1;
}
