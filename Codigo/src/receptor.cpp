// Placa B - receptor.
// Sincroniza no 0xAA, le n e depois exatamente 16*n bytes com tempo limite
// (quadro.c), decifra em ECB, valida e remove o PKCS#7 e mostra o texto
// original no monitor serial (USB).

#include <Arduino.h>

#include <cstring>
#include <strings.h>

#include "comum.h"
#include "pkcs7_ecb.h"
#include "quadro.h"

static quadro_rx rx;  // ~4 KB: guarda ate 255 blocos
static uint8_t claro[QUADRO_MAX_DADOS];
static uint32_t quadros_ok = 0, quadros_erro = 0;

static void processar_quadro() {
  const size_t tamanho = rx.esperado;
  ++quadros_ok;
  Serial.printf("[B] QUADRO %lu recebido marcador=0x%02x n=%u bytes=%u descartados_antes=%lu\n",
                (unsigned long)quadros_ok, QUADRO_MARCADOR, (unsigned)rx.n,
                (unsigned)(2 + tamanho), (unsigned long)rx.descartados);
  rx.descartados = 0;
  log_blocos("CIFRADO_RECEBIDO", rx.dados, tamanho);
  ecb_decifrar(&g_ctx, rx.dados, claro, rx.n);
  log_blocos("DECIFRADO_COM_PKCS7", claro, tamanho);
  size_t tamanho_msg = 0;
  const pkcs7_resultado r = pkcs7_validar(claro, tamanho, &tamanho_msg);
  if (r != PKCS7_OK) {
    // Chave errada ou bytes corrompidos costumam gerar padding invalido;
    // mesmo assim padding valido nao garante integridade (ECB nao autentica).
    Serial.printf("[B] PKCS7 status=INVALIDO motivo=%s; mensagem descartada\n", pkcs7_descricao(r));
    comum_mostrar_chave();
    return;
  }
  Serial.printf("[B] PKCS7 status=valido removidos=%u bytes de valor 0x%02x\n",
                (unsigned)(tamanho - tamanho_msg), (unsigned)claro[tamanho - 1]);
  log_texto("MENSAGEM_RECUPERADA", claro, tamanho_msg);
  log_hex("MENSAGEM_RECUPERADA_HEX", claro, tamanho_msg);
  const bool igual = tamanho_msg == g_tamanho_mensagem_dupla &&
                     memcmp(claro, g_mensagem_dupla, tamanho_msg) == 0;
  Serial.printf("[B] COMPARACAO igual_a_mensagem_da_dupla=%s\n", igual ? "SIM" : "NAO");
}

static void tratar_evento(quadro_rx_evento ev) {
  switch (ev) {
    case QRX_NADA: return;
    case QRX_QUADRO_COMPLETO: processar_quadro(); return;
    case QRX_ERRO_N_ZERO:
      ++quadros_erro;
      Serial.println("[B] QUADRO ERRO n=0 invalido; aguardando novo 0xAA");
      return;
    case QRX_ERRO_TIMEOUT:
      ++quadros_erro;
      Serial.printf("[B] QUADRO ERRO timeout recebidos=%u de %u bytes de dados (n=%u); "
                    "quadro incompleto descartado\n",
                    (unsigned)rx.recebidos, (unsigned)rx.esperado, (unsigned)rx.n);
      return;
  }
}

static void ajuda() {
  Serial.println("[B] AJUDA ESTADO | AJUDA");
  comum_ajuda_comum();
}

void setup() {
  // Buffer da UART2 maior que o maior quadro (4.082 bytes).
  comum_iniciar('B', 4352);
  quadro_rx_iniciar(&rx);
  ajuda();
  Serial.printf("[B] PRONTO aguardando 0xAA; timeout entre bytes=%u ms total=%u ms\n",
                QUADRO_TIMEOUT_ENTRE_BYTES_MS, QUADRO_TIMEOUT_TOTAL_MS);
}

void loop() {
  while (enlace.available()) tratar_evento(quadro_rx_byte(&rx, (uint8_t)enlace.read(), millis()));
  tratar_evento(quadro_rx_verificar_timeout(&rx, millis()));
  char linha[MENSAGEM_MAX + 32];
  if (comum_ler_linha(linha, sizeof linha)) {
    if (!strcasecmp(linha, "AJUDA") || !strcasecmp(linha, "HELP")) {
      ajuda();
    } else if (!strcasecmp(linha, "ESTADO")) {
      Serial.printf("[B] ESTADO quadros_ok=%lu quadros_erro=%lu descartados=%lu\n",
                    (unsigned long)quadros_ok, (unsigned long)quadros_erro,
                    (unsigned long)rx.descartados);
    } else if (!comum_tratar_comando(linha)) {
      Serial.printf("[B] CONSOLE ERRO comando_desconhecido \"%s\"; use AJUDA\n", linha);
    }
  }
  delay(1);
}
