#ifndef AES128_H
#define AES128_H

// AES-128 (FIPS-197) implementado do zero, sem bibliotecas criptograficas.
// Independente de Arduino: o mesmo codigo roda nas placas e nos testes do PC.
//
// Convencao de bytes: um bloco de 16 bytes e lido na ordem de entrada
// in[0..15] e vira o estado em colunas, como no FIPS-197 secao 3.4:
//   s[linha][coluna] = in[linha + 4*coluna]
// Neste codigo o estado fica guardado nesse mesmo vetor linear de 16 bytes,
// entao todo registro em hexadecimal segue a ordem de entrada/saida.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AES_BLOCO_BYTES 16
#define AES128_CHAVE_BYTES 16
#define AES128_RODADAS 10

typedef struct {
  // Chaves de rodada 0..10, cada uma com 16 bytes (palavras w[4i..4i+3]).
  uint8_t chaves_rodada[AES128_RODADAS + 1][AES_BLOCO_BYTES];
} aes128_ctx;

// S-box e inversa, geradas por aes_gerar_tabelas() a partir do corpo GF(2^8).
extern uint8_t AES_SBOX[256];
extern uint8_t AES_SBOX_INV[256];

// Gera as tabelas uma unica vez. aes128_iniciar() chama automaticamente.
void aes_gerar_tabelas(void);

// Aritmetica em GF(2^8) com polinomio irredutivel x^8 + x^4 + x^3 + x + 1.
uint8_t aes_xtime(uint8_t a);
uint8_t aes_gmul(uint8_t a, uint8_t b);

// Transformacoes da rodada, expostas para explicacao e testes.
void aes_sub_bytes(uint8_t s[16]);
void aes_inv_sub_bytes(uint8_t s[16]);
void aes_shift_rows(uint8_t s[16]);
void aes_inv_shift_rows(uint8_t s[16]);
void aes_mix_columns(uint8_t s[16]);
void aes_inv_mix_columns(uint8_t s[16]);
void aes_add_round_key(uint8_t s[16], const uint8_t chave_rodada[16]);

// Expansao de chave: 16 bytes -> 11 chaves de rodada.
void aes128_iniciar(aes128_ctx *ctx, const uint8_t chave[16]);

void aes128_cifrar_bloco(const aes128_ctx *ctx, const uint8_t in[16], uint8_t out[16]);
void aes128_decifrar_bloco(const aes128_ctx *ctx, const uint8_t in[16], uint8_t out[16]);

// Etapa 3 do enunciado: cinco estados do bloco na rodada 1.
// estados[0] AddRoundKey inicial (chave 0), [1] SubBytes, [2] ShiftRows,
// [3] MixColumns, [4] AddRoundKey (chave 1).
void aes128_estados_rodada1(const aes128_ctx *ctx, const uint8_t in[16],
                            uint8_t estados[5][16]);

#ifdef __cplusplus
}
#endif

#endif
