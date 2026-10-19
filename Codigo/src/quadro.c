#include "quadro.h"

#include <string.h>

size_t quadro_montar(const uint8_t *cifrado, size_t blocos, uint8_t *saida, size_t capacidade) {
  if (blocos == 0 || blocos > QUADRO_MAX_BLOCOS) return 0;
  const size_t total = 2 + 16 * blocos;
  if (total > capacidade) return 0;
  saida[0] = QUADRO_MARCADOR;
  saida[1] = (uint8_t)blocos;
  memcpy(saida + 2, cifrado, 16 * blocos);
  return total;
}

void quadro_rx_iniciar(quadro_rx *rx) {
  rx->estado = QRX_AGUARDA_MARCADOR;
  rx->n = 0;
  rx->esperado = rx->recebidos = 0;
  rx->t_inicio_ms = rx->t_ultimo_ms = 0;
  rx->descartados = 0;
}

quadro_rx_evento quadro_rx_byte(quadro_rx *rx, uint8_t byte, uint32_t agora_ms) {
  // Um byte atrasado demais pertence a outro quadro: abandona o atual antes.
  quadro_rx_evento ev = quadro_rx_verificar_timeout(rx, agora_ms);
  if (ev != QRX_NADA && byte != QUADRO_MARCADOR) {
    ++rx->descartados;
    return ev;
  }
  switch (rx->estado) {
    case QRX_AGUARDA_MARCADOR:
      if (byte == QUADRO_MARCADOR) {
        rx->estado = QRX_AGUARDA_N;
        rx->t_inicio_ms = rx->t_ultimo_ms = agora_ms;
      } else {
        ++rx->descartados;
      }
      return ev;
    case QRX_AGUARDA_N:
      rx->t_ultimo_ms = agora_ms;
      if (byte == 0) {
        rx->estado = QRX_AGUARDA_MARCADOR;
        return QRX_ERRO_N_ZERO;
      }
      rx->n = byte;
      rx->esperado = 16u * byte;
      rx->recebidos = 0;
      rx->estado = QRX_RECEBENDO;
      return ev;
    case QRX_RECEBENDO:
      rx->t_ultimo_ms = agora_ms;
      rx->dados[rx->recebidos++] = byte;
      if (rx->recebidos == rx->esperado) {
        rx->estado = QRX_AGUARDA_MARCADOR;
        return QRX_QUADRO_COMPLETO;
      }
      return ev;
  }
  return ev;
}

quadro_rx_evento quadro_rx_verificar_timeout(quadro_rx *rx, uint32_t agora_ms) {
  if (rx->estado == QRX_AGUARDA_MARCADOR) return QRX_NADA;
  if ((uint32_t)(agora_ms - rx->t_ultimo_ms) > QUADRO_TIMEOUT_ENTRE_BYTES_MS ||
      (uint32_t)(agora_ms - rx->t_inicio_ms) > QUADRO_TIMEOUT_TOTAL_MS) {
    rx->estado = QRX_AGUARDA_MARCADOR;
    return QRX_ERRO_TIMEOUT;
  }
  return QRX_NADA;
}
