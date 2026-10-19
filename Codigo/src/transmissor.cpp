// Placa A - transmissor.
// Cifra a mensagem com AES-128 em ECB (PKCS#7) e envia pela UART2 o quadro
// binario 0xAA | n | 16*n bytes, com write() de bytes brutos.
//
// Envio: uma vez 3 s apos o boot, depois a cada ENVIAR na USB ou a cada
// toque no botao BOOT (GPIO0) da placa. MSG <texto> cifra e envia um texto
// novo (alteracao proposta na arguicao).

#include <Arduino.h>

#include <cstring>
#include <strings.h>

#include "analise.h"
#include "comum.h"
#include "quadro.h"

static constexpr uint32_t kEnvioInicialMs = 3000;
static constexpr int kPinoBotaoBoot = 0;  // botao BOOT da placa de desenvolvimento

static uint8_t mensagem[MENSAGEM_MAX];
static size_t tamanho_mensagem = 0;
static uint32_t envios = 0;

static void usar_mensagem_da_dupla() {
  memcpy(mensagem, g_mensagem_dupla, g_tamanho_mensagem_dupla);
  tamanho_mensagem = g_tamanho_mensagem_dupla;
}

static void enviar() {
  if (!g_autoteste_ok) {
    Serial.println("[A] TX RECUSADO autoteste_falhou; corrija antes de transmitir");
    return;
  }
  static uint8_t preenchida[256], cifrado[256], quadro[2 + 256];
  size_t tamanho = 0;
  if (analise_cifrar_mensagem(&g_ctx, mensagem, tamanho_mensagem, preenchida, cifrado,
                              sizeof cifrado, &tamanho) != PKCS7_OK) {
    Serial.println("[A] TX ERRO mensagem_grande_demais");
    return;
  }
  const size_t blocos = tamanho / 16;
  const size_t tamanho_quadro = quadro_montar(cifrado, blocos, quadro, sizeof quadro);
  ++envios;
  Serial.printf("[A] ENVIO %lu inicio\n", (unsigned long)envios);
  comum_mostrar_chave();
  log_texto("MENSAGEM_ORIGINAL", mensagem, tamanho_mensagem);
  Serial.printf("[A] PKCS7 acrescentados=%u bytes de valor 0x%02x tamanho_final=%u\n",
                (unsigned)(tamanho - tamanho_mensagem), (unsigned)(tamanho - tamanho_mensagem),
                (unsigned)tamanho);
  log_blocos("MENSAGEM_PKCS7", preenchida, tamanho);
  log_blocos("CIFRADO_ENVIADO", cifrado, tamanho);
  log_hex("QUADRO_TX", quadro, tamanho_quadro);
  // Bytes crus na UART2; flush() espera o ultimo byte sair do hardware.
  const size_t escritos = enlace.write(quadro, tamanho_quadro);
  enlace.flush();
  Serial.printf("[A] ENVIO %lu fim bytes=%u escritos=%u blocos=%u marcador=0x%02x\n",
                (unsigned long)envios, (unsigned)tamanho_quadro, (unsigned)escritos,
                (unsigned)blocos, QUADRO_MARCADOR);
}

static void ajuda() {
  Serial.println("[A] AJUDA ENVIAR | MSG <texto> | MSGRESET | MENSAGEM | AJUDA | botao BOOT = ENVIAR");
  comum_ajuda_comum();
}

static void tratar_linha(const char *linha) {
  if (!strcasecmp(linha, "ENVIAR")) {
    enviar();
  } else if (!strncasecmp(linha, "MSG ", 4)) {
    const size_t n = strlen(linha + 4);
    if (n == 0 || n > MENSAGEM_MAX) {
      Serial.printf("[A] CONSOLE ERRO MSG aceita 1..%d caracteres\n", MENSAGEM_MAX);
      return;
    }
    memcpy(mensagem, linha + 4, n);
    tamanho_mensagem = n;
    Serial.println("[A] MSG mensagem nova definida; enviando");
    enviar();
  } else if (!strcasecmp(linha, "MSGRESET")) {
    usar_mensagem_da_dupla();
    log_texto("MSGRESET mensagem_da_dupla", mensagem, tamanho_mensagem);
  } else if (!strcasecmp(linha, "MENSAGEM")) {
    log_texto("MENSAGEM_ATUAL", mensagem, tamanho_mensagem);
  } else if (!strcasecmp(linha, "AJUDA") || !strcasecmp(linha, "HELP")) {
    ajuda();
  } else if (!comum_tratar_comando(linha)) {
    Serial.printf("[A] CONSOLE ERRO comando_desconhecido \"%s\"; use AJUDA\n", linha);
  }
}

void setup() {
  comum_iniciar('A', 256);
  pinMode(kPinoBotaoBoot, INPUT_PULLUP);
  usar_mensagem_da_dupla();
  comum_registros_etapa3(mensagem, tamanho_mensagem);
  comum_avalanche(true, 0, mensagem, tamanho_mensagem);
  comum_avalanche(false, 0, mensagem, tamanho_mensagem);
  comum_experimento_ecb();
  ajuda();
  Serial.printf("[A] PRONTO envio automatico em %lu ms\n", (unsigned long)kEnvioInicialMs);
}

void loop() {
  static bool envio_inicial_feito = false;
  if (!envio_inicial_feito && millis() >= kEnvioInicialMs) {
    envio_inicial_feito = true;
    enviar();
  }
  // Botao BOOT: ativo em nivel baixo; dispara no aperto, com 50 ms de filtro.
  static bool apertado = false;
  static uint32_t mudou_em = 0;
  const bool nivel_baixo = digitalRead(kPinoBotaoBoot) == LOW;
  if (nivel_baixo != apertado && millis() - mudou_em > 50) {
    apertado = nivel_baixo;
    mudou_em = millis();
    if (apertado) {
      Serial.println("[A] BOTAO_BOOT pressionado");
      enviar();
    }
  }
  char linha[MENSAGEM_MAX + 32];
  if (comum_ler_linha(linha, sizeof linha)) tratar_linha(linha);
  while (enlace.available()) enlace.read();  // A nao espera dados de B
  delay(1);
}
