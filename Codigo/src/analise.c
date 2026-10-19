#include "analise.h"

void analise_inverter_bit(uint8_t *dados, size_t bit) {
  dados[bit / 8] ^= (uint8_t)(0x80u >> (bit % 8));
}

size_t analise_bits_diferentes(const uint8_t *a, const uint8_t *b, size_t tamanho) {
  size_t total = 0;
  for (size_t i = 0; i < tamanho; ++i) {
    uint8_t x = (uint8_t)(a[i] ^ b[i]);
    while (x) {  // conta os bits 1 do XOR
      total += x & 1u;
      x >>= 1;
    }
  }
  return total;
}

pkcs7_resultado analise_cifrar_mensagem(const aes128_ctx *ctx, const uint8_t *msg,
                                        size_t tamanho, uint8_t *preenchida,
                                        uint8_t *cifrado, size_t capacidade,
                                        size_t *tamanho_cifrado) {
  size_t total = 0;
  const pkcs7_resultado r = pkcs7_preencher(msg, tamanho, preenchida, capacidade, &total);
  if (r != PKCS7_OK) return r;
  ecb_cifrar(ctx, preenchida, cifrado, total / AES_BLOCO_BYTES);
  *tamanho_cifrado = total;
  return PKCS7_OK;
}
