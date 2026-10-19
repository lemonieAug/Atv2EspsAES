#ifndef VETORES_PDF_H
#define VETORES_PDF_H

// Valores esperados transcritos de AES_Trabalho.pdf (paginas 1 e 2), os
// mesmos de Codigo/testes/vetores.json. Usados no autoteste das placas e
// nos testes do computador; nao sao resultados de execucao.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *nome;
  uint8_t chave[16];
  uint8_t claro[16];
  uint8_t cifrado[16];
} vetor_bloco;

// FIPS-197 Apendices B e C.1 (tabela da pagina 2 do enunciado).
extern const vetor_bloco VETORES_FIPS[2];

// Exemplo completo da secao 2 (matriculas ficticias 20231001 e 20231002).
extern const char EXEMPLO_MATRICULA_A[];
extern const char EXEMPLO_MATRICULA_B[];
extern const uint8_t EXEMPLO_CHAVE[16];
extern const char EXEMPLO_MENSAGEM[];
extern const uint8_t EXEMPLO_PREENCHIDA[32];
extern const uint8_t EXEMPLO_CIFRADO[32];

// Experimento ECB da pagina 2: tres blocos com a chave do exemplo.
extern const char *const ECB_CLAROS[3];
extern const uint8_t ECB_CIFRADOS[3][16];

#ifdef __cplusplus
}
#endif

#endif
