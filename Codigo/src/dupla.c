#include "dupla.h"

#include <stdio.h>
#include <string.h>

// Escreve em out os ultimos 8 digitos de matricula, com '0' a esquerda.
static int ultimos_oito_digitos(const char *matricula, uint8_t out[8]) {
  char digitos[8];
  size_t total = 0;
  for (const char *p = matricula; *p; ++p) {
    if (*p < '0' || *p > '9') continue;  // so os digitos entram na chave
    if (total < 8) digitos[total] = *p;
    else {
      memmove(digitos, digitos + 1, 7);  // mantem a janela dos 8 ultimos
      digitos[7] = *p;
    }
    ++total;
  }
  if (total == 0) return -1;
  const size_t usados = total < 8 ? total : 8;
  memset(out, '0', 8 - usados);
  memcpy(out + (8 - usados), digitos, usados);
  return 0;
}

int dupla_derivar_chave(const char *matricula_a, const char *matricula_b, uint8_t chave[16]) {
  if (ultimos_oito_digitos(matricula_a, chave) != 0) return -1;
  if (ultimos_oito_digitos(matricula_b, chave + 8) != 0) return -1;
  return 0;
}

size_t dupla_montar_mensagem(const char *matricula_a, const char *matricula_b, char *saida,
                             size_t capacidade) {
  const int n = snprintf(saida, capacidade, "A:%s;B:%s", matricula_a, matricula_b);
  if (n < 0 || (size_t)n >= capacidade) return 0;
  return (size_t)n;
}
