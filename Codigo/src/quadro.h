#ifndef QUADRO_H
#define QUADRO_H

// Quadro binario do enunciado na UART2:
//   0xAA (marcador) | n (1 byte, numero de blocos) | exatamente 16*n bytes cifrados
// Os bytes vao crus (write), nunca como texto hexadecimal.

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QUADRO_MARCADOR 0xAA
#define QUADRO_MAX_BLOCOS 255  // limite do campo n de 1 byte
#define QUADRO_MAX_DADOS (QUADRO_MAX_BLOCOS * 16)
#define QUADRO_MAX_BYTES (2 + QUADRO_MAX_DADOS)

// Tempo limite do receptor depois do marcador. A 115200 bps (8N1, ~87 us por
// byte) o maior quadro (4.082 bytes) leva ~355 ms, logo os limites folgam.
#define QUADRO_TIMEOUT_ENTRE_BYTES_MS 100u
#define QUADRO_TIMEOUT_TOTAL_MS 1000u

// Monta o quadro em saida. Retorna o tamanho, ou 0 se blocos for 0 ou maior
// que 255, ou se capacidade for pequena.
size_t quadro_montar(const uint8_t *cifrado, size_t blocos, uint8_t *saida, size_t capacidade);

typedef enum {
  QRX_AGUARDA_MARCADOR = 0,  // descarta tudo ate achar 0xAA
  QRX_AGUARDA_N,             // proximo byte e o numero de blocos
  QRX_RECEBENDO              // le exatamente 16*n bytes
} quadro_rx_estado;

typedef enum {
  QRX_NADA = 0,
  QRX_QUADRO_COMPLETO,  // n e dados validos em rx->n / rx->dados
  QRX_ERRO_N_ZERO,      // n = 0 e impossivel com PKCS#7 (sempre >= 1 bloco)
  QRX_ERRO_TIMEOUT      // quadro incompleto abandonado; volta a sincronizar
} quadro_rx_evento;

typedef struct {
  quadro_rx_estado estado;
  uint8_t n;
  size_t esperado, recebidos;
  uint32_t t_inicio_ms, t_ultimo_ms;
  uint32_t descartados;  // bytes ignorados antes de um marcador
  uint8_t dados[QUADRO_MAX_DADOS];
} quadro_rx;

void quadro_rx_iniciar(quadro_rx *rx);

// Entrega um byte recebido. agora_ms e um relogio monotono em milissegundos.
quadro_rx_evento quadro_rx_byte(quadro_rx *rx, uint8_t byte, uint32_t agora_ms);

// Deve ser chamada periodicamente, mesmo sem bytes, para aplicar o timeout.
quadro_rx_evento quadro_rx_verificar_timeout(quadro_rx *rx, uint32_t agora_ms);

#ifdef __cplusplus
}
#endif

#endif
