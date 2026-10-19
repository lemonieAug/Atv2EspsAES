#include "aes128.h"

#include <string.h>

uint8_t AES_SBOX[256];
uint8_t AES_SBOX_INV[256];
static int tabelas_prontas = 0;

// Multiplicar por x (0x02) em GF(2^8): desloca 1 bit e, se o bit 7 saiu,
// reduz pelo polinomio do AES (0x11b), o que equivale a XOR com 0x1b.
uint8_t aes_xtime(uint8_t a) {
  return (uint8_t)((a << 1) ^ ((a & 0x80) ? 0x1b : 0x00));
}

// Multiplicacao geral por somas de xtime: para cada bit de b, soma (XOR) a*x^i.
uint8_t aes_gmul(uint8_t a, uint8_t b) {
  uint8_t r = 0;
  while (b) {
    if (b & 1) r ^= a;
    a = aes_xtime(a);
    b >>= 1;
  }
  return r;
}

static uint8_t rotl8(uint8_t x, int n) {
  return (uint8_t)((x << n) | (x >> (8 - n)));
}

// Origem da S-box (FIPS-197 secao 5.1.1): para cada byte x,
// 1) toma o inverso multiplicativo em GF(2^8) (0 vai para 0);
// 2) aplica a transformacao afim b ^ rotl(b,1..4) ^ 0x63.
// O inverso e x^254, pois x^255 = 1 para todo x != 0 no corpo de 256 elementos.
// A S-box inversa e a tabela que desfaz essa permutacao.
void aes_gerar_tabelas(void) {
  if (tabelas_prontas) return;
  for (int x = 0; x < 256; ++x) {
    uint8_t inv = 0;
    if (x) {
      uint8_t p = 1, base = (uint8_t)x;
      for (int e = 254; e; e >>= 1) {  // exponenciacao rapida: x^254
        if (e & 1) p = aes_gmul(p, base);
        base = aes_gmul(base, base);
      }
      inv = p;
    }
    const uint8_t s = (uint8_t)(inv ^ rotl8(inv, 1) ^ rotl8(inv, 2) ^ rotl8(inv, 3) ^
                                rotl8(inv, 4) ^ 0x63);
    AES_SBOX[x] = s;
    AES_SBOX_INV[s] = (uint8_t)x;
  }
  tabelas_prontas = 1;
}

// SubBytes: substituicao nao linear byte a byte (confusao).
void aes_sub_bytes(uint8_t s[16]) {
  for (int i = 0; i < 16; ++i) s[i] = AES_SBOX[s[i]];
}

void aes_inv_sub_bytes(uint8_t s[16]) {
  for (int i = 0; i < 16; ++i) s[i] = AES_SBOX_INV[s[i]];
}

// ShiftRows: a linha r gira r posicoes para a esquerda, espalhando os bytes
// de uma coluna por colunas diferentes (difusao entre colunas).
// Com s[r][c] = s[r + 4c], o novo s[r][c] e o antigo s[r][(c + r) mod 4].
void aes_shift_rows(uint8_t s[16]) {
  uint8_t t[16];
  for (int c = 0; c < 4; ++c)
    for (int r = 0; r < 4; ++r) t[r + 4 * c] = s[r + 4 * ((c + r) % 4)];
  memcpy(s, t, 16);
}

void aes_inv_shift_rows(uint8_t s[16]) {
  uint8_t t[16];
  for (int c = 0; c < 4; ++c)
    for (int r = 0; r < 4; ++r) t[r + 4 * ((c + r) % 4)] = s[r + 4 * c];
  memcpy(s, t, 16);
}

// MixColumns: cada coluna e multiplicada pela matriz fixa
// [02 03 01 01; 01 02 03 01; 01 01 02 03; 03 01 01 02] em GF(2^8),
// misturando os 4 bytes da coluna (difusao dentro da coluna).
// 02*a = xtime(a); 03*a = xtime(a) ^ a.
void aes_mix_columns(uint8_t s[16]) {
  for (int c = 0; c < 4; ++c) {
    uint8_t *col = s + 4 * c;
    const uint8_t a0 = col[0], a1 = col[1], a2 = col[2], a3 = col[3];
    col[0] = (uint8_t)(aes_xtime(a0) ^ (aes_xtime(a1) ^ a1) ^ a2 ^ a3);
    col[1] = (uint8_t)(a0 ^ aes_xtime(a1) ^ (aes_xtime(a2) ^ a2) ^ a3);
    col[2] = (uint8_t)(a0 ^ a1 ^ aes_xtime(a2) ^ (aes_xtime(a3) ^ a3));
    col[3] = (uint8_t)((aes_xtime(a0) ^ a0) ^ a1 ^ a2 ^ aes_xtime(a3));
  }
}

// Inversa: matriz [0e 0b 0d 09; 09 0e 0b 0d; 0d 09 0e 0b; 0b 0d 09 0e].
void aes_inv_mix_columns(uint8_t s[16]) {
  for (int c = 0; c < 4; ++c) {
    uint8_t *col = s + 4 * c;
    const uint8_t a0 = col[0], a1 = col[1], a2 = col[2], a3 = col[3];
    col[0] = (uint8_t)(aes_gmul(a0, 14) ^ aes_gmul(a1, 11) ^ aes_gmul(a2, 13) ^ aes_gmul(a3, 9));
    col[1] = (uint8_t)(aes_gmul(a0, 9) ^ aes_gmul(a1, 14) ^ aes_gmul(a2, 11) ^ aes_gmul(a3, 13));
    col[2] = (uint8_t)(aes_gmul(a0, 13) ^ aes_gmul(a1, 9) ^ aes_gmul(a2, 14) ^ aes_gmul(a3, 11));
    col[3] = (uint8_t)(aes_gmul(a0, 11) ^ aes_gmul(a1, 13) ^ aes_gmul(a2, 9) ^ aes_gmul(a3, 14));
  }
}

// AddRoundKey: XOR do estado com a chave da rodada; e o unico passo que
// usa a chave. XOR e a propria inversa, entao serve para cifrar e decifrar.
void aes_add_round_key(uint8_t s[16], const uint8_t chave_rodada[16]) {
  for (int i = 0; i < 16; ++i) s[i] ^= chave_rodada[i];
}

// Expansao de chave AES-128 (FIPS-197 secao 5.2): 44 palavras de 4 bytes.
// w[0..3] = chave; para i >= 4, temp = w[i-1]; a cada 4 palavras aplica
// RotWord (gira 1 byte), SubWord (S-box) e XOR com Rcon (potencias de x);
// depois w[i] = w[i-4] ^ temp. Rcon quebra a simetria entre as rodadas.
void aes128_iniciar(aes128_ctx *ctx, const uint8_t chave[16]) {
  aes_gerar_tabelas();
  uint8_t w[44][4];
  memcpy(w, chave, 16);
  uint8_t rcon = 0x01;
  for (int i = 4; i < 44; ++i) {
    uint8_t t[4] = {w[i - 1][0], w[i - 1][1], w[i - 1][2], w[i - 1][3]};
    if (i % 4 == 0) {
      const uint8_t primeiro = t[0];  // RotWord
      t[0] = t[1]; t[1] = t[2]; t[2] = t[3]; t[3] = primeiro;
      for (int k = 0; k < 4; ++k) t[k] = AES_SBOX[t[k]];  // SubWord
      t[0] ^= rcon;
      rcon = aes_xtime(rcon);
    }
    for (int k = 0; k < 4; ++k) w[i][k] = (uint8_t)(w[i - 4][k] ^ t[k]);
  }
  for (int r = 0; r <= AES128_RODADAS; ++r) memcpy(ctx->chaves_rodada[r], w[4 * r], 16);
}

// Cifra: AddRoundKey(0); rodadas 1..9 completas; rodada 10 sem MixColumns.
void aes128_cifrar_bloco(const aes128_ctx *ctx, const uint8_t in[16], uint8_t out[16]) {
  uint8_t s[16];
  memcpy(s, in, 16);
  aes_add_round_key(s, ctx->chaves_rodada[0]);
  for (int r = 1; r <= AES128_RODADAS; ++r) {
    aes_sub_bytes(s);
    aes_shift_rows(s);
    if (r != AES128_RODADAS) aes_mix_columns(s);
    aes_add_round_key(s, ctx->chaves_rodada[r]);
  }
  memcpy(out, s, 16);
}

// Decifra: as inversas na ordem contraria, com as chaves de 10 ate 0.
void aes128_decifrar_bloco(const aes128_ctx *ctx, const uint8_t in[16], uint8_t out[16]) {
  uint8_t s[16];
  memcpy(s, in, 16);
  aes_add_round_key(s, ctx->chaves_rodada[AES128_RODADAS]);
  for (int r = AES128_RODADAS - 1; r >= 0; --r) {
    aes_inv_shift_rows(s);
    aes_inv_sub_bytes(s);
    aes_add_round_key(s, ctx->chaves_rodada[r]);
    if (r != 0) aes_inv_mix_columns(s);
  }
  memcpy(out, s, 16);
}

void aes128_estados_rodada1(const aes128_ctx *ctx, const uint8_t in[16],
                            uint8_t estados[5][16]) {
  uint8_t s[16];
  memcpy(s, in, 16);
  aes_add_round_key(s, ctx->chaves_rodada[0]);
  memcpy(estados[0], s, 16);
  aes_sub_bytes(s);
  memcpy(estados[1], s, 16);
  aes_shift_rows(s);
  memcpy(estados[2], s, 16);
  aes_mix_columns(s);
  memcpy(estados[3], s, 16);
  aes_add_round_key(s, ctx->chaves_rodada[1]);
  memcpy(estados[4], s, 16);
}
