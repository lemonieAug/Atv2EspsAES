#ifndef ANALISE_H
#define ANALISE_H

// Funcoes auxiliares da Etapa 3: cifrar uma mensagem inteira (PKCS#7 + ECB)
// e medir o efeito avalanche.

#include <stddef.h>
#include <stdint.h>

#include "aes128.h"
#include "pkcs7_ecb.h"

#ifdef __cplusplus
extern "C" {
#endif

// Numeracao de bits: o bit i fica no byte i/8, contando do mais significativo
// (bit 0 = 0x80 do byte 0, bit 7 = 0x01 do byte 0, bit 8 = 0x80 do byte 1...).
void analise_inverter_bit(uint8_t *dados, size_t bit);
size_t analise_bits_diferentes(const uint8_t *a, const uint8_t *b, size_t tamanho);

// preenchida recebe a mensagem com PKCS#7 e cifrado o resultado em ECB;
// ambos precisam de pkcs7_tamanho_preenchido(tamanho) bytes.
pkcs7_resultado analise_cifrar_mensagem(const aes128_ctx *ctx, const uint8_t *msg,
                                        size_t tamanho, uint8_t *preenchida,
                                        uint8_t *cifrado, size_t capacidade,
                                        size_t *tamanho_cifrado);

#ifdef __cplusplus
}
#endif

#endif
