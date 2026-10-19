#ifndef PKCS7_ECB_H
#define PKCS7_ECB_H

// Preenchimento PKCS#7 e modo ECB sobre o AES-128 deste projeto.

#include <stddef.h>
#include <stdint.h>

#include "aes128.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  PKCS7_OK = 0,
  PKCS7_ERRO_CAPACIDADE,    // buffer de saida pequeno demais
  PKCS7_ERRO_TAMANHO,       // tamanho zero ou nao multiplo de 16
  PKCS7_ERRO_VALOR,         // ultimo byte 0x00 ou maior que 16
  PKCS7_ERRO_BYTES          // bytes de preenchimento diferentes entre si
} pkcs7_resultado;

// Sempre acrescenta de 1 a 16 bytes; uma entrada multipla de 16 ganha
// um bloco inteiro de 0x10, para que a remocao nunca seja ambigua.
size_t pkcs7_tamanho_preenchido(size_t tamanho);
pkcs7_resultado pkcs7_preencher(const uint8_t *msg, size_t tamanho, uint8_t *saida,
                                size_t capacidade, size_t *tamanho_saida);

// Valida o preenchimento e informa o tamanho da mensagem original.
// Padding valido NAO prova chave correta nem integridade: ECB nao autentica.
pkcs7_resultado pkcs7_validar(const uint8_t *dados, size_t tamanho, size_t *tamanho_msg);
const char *pkcs7_descricao(pkcs7_resultado r);

// ECB: cada bloco de 16 bytes e cifrado independentemente com a mesma chave.
// Por isso blocos claros iguais geram blocos cifrados iguais.
void ecb_cifrar(const aes128_ctx *ctx, const uint8_t *in, uint8_t *out, size_t blocos);
void ecb_decifrar(const aes128_ctx *ctx, const uint8_t *in, uint8_t *out, size_t blocos);

#ifdef __cplusplus
}
#endif

#endif
