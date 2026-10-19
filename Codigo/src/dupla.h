#ifndef DUPLA_H
#define DUPLA_H

// Dados da dupla e regras do enunciado (secao 2) para chave e mensagem.
//
// Matriculas reais da dupla, informadas pelo usuario em 19/10/2026, na ordem
// A (transmissor) e B (receptor). O mesmo arquivo vale para as duas placas:
// apos qualquer alteracao, grave esp_a e esp_b novamente.
// DUPLA_DADOS_FICTICIOS = 1 apenas quando usar as matriculas do exemplo do PDF.
#define DUPLA_MATRICULA_A "2023178440013"
#define DUPLA_MATRICULA_B "202127844251"
#define DUPLA_DADOS_FICTICIOS 0

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Chave: so os digitos; ultimos 8 de A (completando com '0' a esquerda),
// depois os ultimos 8 de B. Cada caractere vira seu codigo ASCII ('2' = 0x32).
// Retorna 0 em sucesso e -1 se alguma matricula nao tiver digitos.
int dupla_derivar_chave(const char *matricula_a, const char *matricula_b, uint8_t chave[16]);

// Mensagem "A:<matricula A>;B:<matricula B>" com as matriculas completas.
// Retorna o tamanho, ou 0 se nao couber em capacidade (inclui o '\0').
size_t dupla_montar_mensagem(const char *matricula_a, const char *matricula_b, char *saida,
                             size_t capacidade);

#ifdef __cplusplus
}
#endif

#endif
