#include "pkcs7_ecb.h"

#include <string.h>

size_t pkcs7_tamanho_preenchido(size_t tamanho) {
  return (tamanho / AES_BLOCO_BYTES + 1) * AES_BLOCO_BYTES;
}

pkcs7_resultado pkcs7_preencher(const uint8_t *msg, size_t tamanho, uint8_t *saida,
                                size_t capacidade, size_t *tamanho_saida) {
  const size_t total = pkcs7_tamanho_preenchido(tamanho);
  if (total > capacidade) return PKCS7_ERRO_CAPACIDADE;
  const uint8_t falta = (uint8_t)(total - tamanho);  // 1..16
  if (tamanho) memmove(saida, msg, tamanho);
  memset(saida + tamanho, falta, falta);
  *tamanho_saida = total;
  return PKCS7_OK;
}

pkcs7_resultado pkcs7_validar(const uint8_t *dados, size_t tamanho, size_t *tamanho_msg) {
  if (tamanho == 0 || tamanho % AES_BLOCO_BYTES) return PKCS7_ERRO_TAMANHO;
  const uint8_t p = dados[tamanho - 1];
  if (p == 0 || p > AES_BLOCO_BYTES) return PKCS7_ERRO_VALOR;
  for (size_t i = tamanho - p; i < tamanho; ++i)
    if (dados[i] != p) return PKCS7_ERRO_BYTES;
  *tamanho_msg = tamanho - p;
  return PKCS7_OK;
}

const char *pkcs7_descricao(pkcs7_resultado r) {
  switch (r) {
    case PKCS7_OK: return "valido";
    case PKCS7_ERRO_CAPACIDADE: return "buffer_insuficiente";
    case PKCS7_ERRO_TAMANHO: return "tamanho_invalido";
    case PKCS7_ERRO_VALOR: return "valor_de_preenchimento_invalido";
    case PKCS7_ERRO_BYTES: return "bytes_de_preenchimento_inconsistentes";
  }
  return "desconhecido";
}

void ecb_cifrar(const aes128_ctx *ctx, const uint8_t *in, uint8_t *out, size_t blocos) {
  for (size_t b = 0; b < blocos; ++b)
    aes128_cifrar_bloco(ctx, in + 16 * b, out + 16 * b);
}

void ecb_decifrar(const aes128_ctx *ctx, const uint8_t *in, uint8_t *out, size_t blocos) {
  for (size_t b = 0; b < blocos; ++b)
    aes128_decifrar_bloco(ctx, in + 16 * b, out + 16 * b);
}
