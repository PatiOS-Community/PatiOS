#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <strings.h>
#include <dirent.h>

#define DIL_YOL      "/pati-services/languages/"
#define HARTA_YOL    "/pati-services/keymaps/"
#define AD_BOY       128
#define SATIR_BOY    512

typedef struct {
    uint8_t  giris;
    uint32_t cikis;
} Esleme;

typedef struct {
    char   ad[AD_BOY];
    char   harta[AD_BOY];
    Esleme tablo[256];
    int    boy;
    int    utf8;
} Dil;

static Dil su_an;
static int hazir = 0;

static void sondur(char *s) {
    int n = (int)strlen(s);
    while (n > 0 && (unsigned char)s[n - 1] <= ' ')
        s[--n] = '\0';
}

static char *bosatla(char *s) {
    while (*s && (unsigned char)*s <= ' ')
        s++;
    return s;
}

static uint32_t heks(const char *s) {
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        s += 2;
    uint32_t deger = 0;
    for (; *s; s++) {
        if      (*s >= '0' && *s <= '9') deger = deger * 16 + (*s - '0');
        else if (*s >= 'a' && *s <= 'f') deger = deger * 16 + (*s - 'a' + 10);
        else if (*s >= 'A' && *s <= 'F') deger = deger * 16 + (*s - 'A' + 10);
        else break;
    }
    return deger;
}

int utf8yaz(uint32_t kod, char *cikti) {
    if (kod < 0x80) {
        cikti[0] = (char)kod;
        cikti[1] = '\0';
        return 1;
    }
    if (kod < 0x800) {
        cikti[0] = (char)(0xC0 | (kod >> 6));
        cikti[1] = (char)(0x80 | (kod & 0x3F));
        cikti[2] = '\0';
        return 2;
    }
    if (kod < 0x10000) {
        cikti[0] = (char)(0xE0 | (kod >> 12));
        cikti[1] = (char)(0x80 | ((kod >> 6) & 0x3F));
        cikti[2] = (char)(0x80 | (kod & 0x3F));
        cikti[3] = '\0';
        return 3;
    }
    cikti[0] = (char)(0xF0 | (kod >> 18));
    cikti[1] = (char)(0x80 | ((kod >> 12) & 0x3F));
    cikti[2] = (char)(0x80 | ((kod >> 6) & 0x3F));
    cikti[3] = (char)(0x80 | (kod & 0x3F));
    cikti[4] = '\0';
    return 4;
}

static void isimden_harta(const char *dil_adi, char *harta_cikti) {
    const char *nokta = strchr(dil_adi, '.');
    if (nokta && *(nokta + 1)) {
        strncpy(harta_cikti, nokta + 1, AD_BOY - 1);
        harta_cikti[AD_BOY - 1] = '\0';
    } else {
        harta_cikti[0] = '\0';
    }
}

static int dil_oku(const char *ad, char *harta_cikti) {
    char yol[256];
    snprintf(yol, sizeof(yol), "%s%s", DIL_YOL, ad);

    FILE *dosya = fopen(yol, "r");
    if (!dosya) return -1;

    char satir[SATIR_BOY];
    int bulundu = 0;

    while (fgets(satir, sizeof(satir), dosya) && !bulundu) {
        sondur(satir);
        char *s = bosatla(satir);
        if (*s == '%' || *s == '\0') continue;

        if (strncmp(s, "charmap", 7) == 0 && (s[7] == ' ' || s[7] == '\t')) {
            strncpy(harta_cikti, bosatla(s + 7), AD_BOY - 1);
            harta_cikti[AD_BOY - 1] = '\0';
            bulundu = 1;
        } else if (strncmp(s, "codeset", 7) == 0) {
            char *deger = bosatla(s + 7);
            if (*deger == '"') deger++;
            char *son = strchr(deger, '"');
            if (son) *son = '\0';
            strncpy(harta_cikti, deger, AD_BOY - 1);
            harta_cikti[AD_BOY - 1] = '\0';
            bulundu = 1;
        }
    }
    fclose(dosya);
    return bulundu ? 0 : -1;
}

static int harta_oku(const char *harta_adi) {
    char yol[256];
    snprintf(yol, sizeof(yol), "%s%s", HARTA_YOL, harta_adi);

    FILE *dosya = fopen(yol, "r");
    if (!dosya) return -1;

    char satir[SATIR_BOY];
    int blokta = 0, idx = 0;

    while (fgets(satir, sizeof(satir), dosya)) {
        sondur(satir);
        char *s = bosatla(satir);
        if (*s == '%' || *s == '\0') continue;

        if (!blokta) {
            if (strcmp(s, "CHARMAP") == 0) blokta = 1;
            continue;
        }
        if (strncmp(s, "END CHARMAP", 11) == 0) break;
        if (s[0] != '<' || s[1] != 'U') continue;
        if (idx >= 256) continue;

        char *son_aci = strchr(s, '>');
        if (!son_aci) continue;

        uint32_t unicode = heks(s + 2);

        char *kacis = strstr(son_aci, "\\x");
        if (!kacis) continue;

        uint8_t bayt = (uint8_t)heks(kacis + 2);
        su_an.tablo[idx].giris = bayt;
        su_an.tablo[idx].cikis = unicode;
        idx++;
    }
    fclose(dosya);
    su_an.boy = idx;
    return idx > 0 ? 0 : -1;
}

int dil_sec(const char *dil_adi) {
    memset(&su_an, 0, sizeof(Dil));
    hazir = 0;

    strncpy(su_an.ad, dil_adi, AD_BOY - 1);

    char harta[AD_BOY] = {0};
    isimden_harta(dil_adi, harta);

    if (harta[0] == '\0')
        dil_oku(dil_adi, harta);

    if (harta[0] == '\0')
        strncpy(harta, "UTF-8", AD_BOY - 1);

    strncpy(su_an.harta, harta, AD_BOY - 1);

    if (strcasecmp(harta, "UTF-8") == 0 || strcasecmp(harta, "utf8") == 0) {
        su_an.utf8 = 1;
        hazir = 1;
        return 0;
    }

    int sonuc = harta_oku(harta);
    if (sonuc < 0)
        su_an.utf8 = 1;

    hazir = 1;
    return sonuc;
}

int harta_algi(void) {
    if (!hazir) return -1;
    if (su_an.utf8) return 1;
    return su_an.boy > 0 ? 0 : -1;
}

uint32_t bayt_utf32(uint8_t bayt) {
    if (!hazir || su_an.utf8)
        return (uint32_t)bayt;
    for (int i = 0; i < su_an.boy; i++) {
        if (su_an.tablo[i].giris == bayt)
            return su_an.tablo[i].cikis;
    }
    return (uint32_t)bayt;
}

int bayt_utf8(uint8_t bayt, char *cikti) {
    return utf8yaz(bayt_utf32(bayt), cikti);
}

int klavye_isle(uint8_t bayt, char *cikti) {
    return bayt_utf8(bayt, cikti);
}

int dize_cevir(const uint8_t *giris, int boy, char *cikti, int cikti_boy) {
    int toplam = 0;
    char tmp[5];
    for (int i = 0; i < boy && (toplam + 5) < cikti_boy; i++) {
        int n = bayt_utf8(giris[i], tmp);
        memcpy(cikti + toplam, tmp, n);
        toplam += n;
    }
    cikti[toplam] = '\0';
    return toplam;
}

int dil_listele(char liste[][AD_BOY], int maks) {
    DIR *dizin = opendir(DIL_YOL);
    if (!dizin) return -1;

    int sayi = 0;
    struct dirent *giris;
    while ((giris = readdir(dizin)) != NULL && sayi < maks) {
        if (giris->d_name[0] == '.') continue;
        strncpy(liste[sayi], giris->d_name, AD_BOY - 1);
        liste[sayi][AD_BOY - 1] = '\0';
        sayi++;
    }
    closedir(dizin);
    return sayi;
}

const char *dil_adi(void) {
    return hazir ? su_an.ad : NULL;
}

const char *dil_hartasi(void) {
    return hazir ? su_an.harta : NULL;
}

int dil_utf8_mi(void) {
    return hazir && su_an.utf8;
}

int dil_hazir(void) {
    return hazir;
}

void dil_sifirla(void) {
    memset(&su_an, 0, sizeof(Dil));
    hazir = 0;
}
