#pragma once
#include <stdint.h>

#define AD_BOY 128

int         dil_sec(const char *dil_adi);
int         harta_algi(void);
int         utf8yaz(uint32_t kod, char *cikti);
uint32_t    bayt_utf32(uint8_t bayt);
int         bayt_utf8(uint8_t bayt, char *cikti);
int         klavye_isle(uint8_t bayt, char *cikti);
int         dize_cevir(const uint8_t *giris, int boy, char *cikti, int cikti_boy);
int         dil_listele(char liste[][AD_BOY], int maks);
const char *dil_adi(void);
const char *dil_hartasi(void);
int         dil_utf8_mi(void);
int         dil_hazir(void);
void        dil_sifirla(void);
