#ifndef COMUM_H
#define COMUM_H

// Partes compartilhadas pelo transmissor (A) e pelo receptor (B):
// UART2 do enlace, autoteste com os vetores do PDF, registros da Etapa 3,
// efeito avalanche, experimento ECB e console de comandos pela USB.
// Toda saida didatica vai para a USB (Serial/UART0); a UART2 so leva o quadro.

#include <Arduino.h>

#include "aes128.h"

#define FIRMWARE_VERSAO "aes128-ecb-v1"
#define MENSAGEM_MAX 240  // texto aceito em A; com PKCS#7 cabe em 256 bytes

extern HardwareSerial enlace;  // UART2: RX GPIO16, TX GPIO17, 115200 8N1
extern char g_papel;           // 'A' ou 'B'
extern bool g_autoteste_ok;
extern uint8_t g_chave_dupla[16];     // derivada das matriculas (dupla.h)
extern uint8_t g_chave_trabalho[16];  // chave em uso; BITCHAVE altera esta copia
extern aes128_ctx g_ctx;              // expansao de g_chave_trabalho
extern char g_mensagem_dupla[64];
extern size_t g_tamanho_mensagem_dupla;

// Inicia USB, gera chave e mensagem da dupla, roda o autoteste e abre a UART2.
void comum_iniciar(char papel, size_t buffer_rx_enlace);

// Registros: "[A] ROTULO ..." para facilitar a leitura e a comparacao A/B.
void log_inicio(const char *rotulo);
void log_hex(const char *rotulo, const uint8_t *dados, size_t tamanho);
void log_blocos(const char *rotulo, const uint8_t *dados, size_t tamanho);
void log_texto(const char *rotulo, const uint8_t *dados, size_t tamanho);

bool comum_autoteste();
void comum_registros_etapa3(const uint8_t *msg, size_t tamanho);
void comum_avalanche(bool na_chave, size_t bit, const uint8_t *msg, size_t tamanho);
void comum_experimento_ecb();
void comum_mostrar_chave();

// Le uma linha da USB sem bloquear; true quando a linha estiver completa.
bool comum_ler_linha(char *linha, size_t capacidade);
// Trata comandos comuns as duas placas. Retorna false se nao reconhecer.
bool comum_tratar_comando(const char *linha);
void comum_ajuda_comum();

#endif
