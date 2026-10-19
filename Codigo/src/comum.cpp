#include "comum.h"

#include <cstdlib>
#include <cstring>
#include <strings.h>

#include "analise.h"
#include "dupla.h"
#include "pkcs7_ecb.h"
#include "vetores_pdf.h"

#if !defined(CONFIG_IDF_TARGET_ESP32)
#error Este firmware usa a UART2 do ESP32 classico nos GPIO16/17.
#endif

HardwareSerial enlace(2);
char g_papel = '?';
bool g_autoteste_ok = false;
uint8_t g_chave_dupla[16];
uint8_t g_chave_trabalho[16];
aes128_ctx g_ctx;
char g_mensagem_dupla[64];
size_t g_tamanho_mensagem_dupla = 0;

void log_inicio(const char *rotulo) { Serial.printf("[%c] %s", g_papel, rotulo); }

void log_hex(const char *rotulo, const uint8_t *dados, size_t tamanho) {
  log_inicio(rotulo);
  Serial.print(' ');
  for (size_t i = 0; i < tamanho; ++i) Serial.printf("%02x", dados[i]);
  Serial.println();
}

// Um bloco de 16 bytes por linha, em hexadecimal na ordem de transmissao.
void log_blocos(const char *rotulo, const uint8_t *dados, size_t tamanho) {
  for (size_t b = 0; b * 16 < tamanho; ++b) {
    Serial.printf("[%c] %s bloco %u: ", g_papel, rotulo, (unsigned)(b + 1));
    for (size_t i = 0; i < 16 && b * 16 + i < tamanho; ++i) Serial.printf("%02x", dados[b * 16 + i]);
    Serial.println();
  }
}

// Texto entre aspas; bytes nao imprimiveis aparecem como \xHH.
void log_texto(const char *rotulo, const uint8_t *dados, size_t tamanho) {
  log_inicio(rotulo);
  Serial.printf(" tamanho=%u texto=\"", (unsigned)tamanho);
  for (size_t i = 0; i < tamanho; ++i) {
    const uint8_t c = dados[i];
    if (c == '"' || c == '\\') Serial.printf("\\%c", c);
    else if (c >= 32 && c <= 126) Serial.print((char)c);
    else Serial.printf("\\x%02x", c);
  }
  Serial.println('"');
}

static void definir_chave_trabalho(const uint8_t chave[16]) {
  memcpy(g_chave_trabalho, chave, 16);
  aes128_iniciar(&g_ctx, g_chave_trabalho);
}

void comum_mostrar_chave() {
  log_inicio("CHAVE");
  Serial.print(" ascii=\"");
  for (int i = 0; i < 16; ++i) {
    const uint8_t c = g_chave_trabalho[i];
    if (c >= 32 && c <= 126) Serial.print((char)c);
    else Serial.printf("\\x%02x", c);
  }
  Serial.print("\" hex=");
  for (int i = 0; i < 16; ++i) Serial.printf("%02x", g_chave_trabalho[i]);
  const size_t dif = analise_bits_diferentes(g_chave_trabalho, g_chave_dupla, 16);
  Serial.printf(" bits_diferentes_da_chave_da_dupla=%u\n", (unsigned)dif);
}

static bool conferir(const char *nome, const uint8_t *obtido, const uint8_t *esperado, size_t n,
                     int *falhas) {
  const bool ok = memcmp(obtido, esperado, n) == 0;
  if (!ok) ++*falhas;
  Serial.printf("[%c] AUTOTESTE %s %s obtido=", g_papel, ok ? "PASS" : "FAIL", nome);
  for (size_t i = 0; i < n; ++i) Serial.printf("%02x", obtido[i]);
  if (!ok) {
    Serial.print(" esperado=");
    for (size_t i = 0; i < n; ++i) Serial.printf("%02x", esperado[i]);
  }
  Serial.println();
  return ok;
}

// Etapa 1: valida a implementacao com os valores do PDF antes de usar a
// chave da dupla. Executa nesta placa, com este firmware.
bool comum_autoteste() {
  int falhas = 0, total = 0;
  for (const vetor_bloco &v : VETORES_FIPS) {
    aes128_ctx ctx;
    uint8_t c[16], p[16];
    char nome[64];
    aes128_iniciar(&ctx, v.chave);
    aes128_cifrar_bloco(&ctx, v.claro, c);
    snprintf(nome, sizeof nome, "%s_cifrar", v.nome);
    conferir(nome, c, v.cifrado, 16, &falhas);
    aes128_decifrar_bloco(&ctx, v.cifrado, p);
    snprintf(nome, sizeof nome, "%s_decifrar", v.nome);
    conferir(nome, p, v.claro, 16, &falhas);
    total += 2;
  }
  uint8_t chave[16], preenchida[32], cifrado[32], claro[32];
  size_t tam = 0, tam_msg = 0;
  char msg[64];
  dupla_derivar_chave(EXEMPLO_MATRICULA_A, EXEMPLO_MATRICULA_B, chave);
  conferir("exemplo_ficticio_chave", chave, EXEMPLO_CHAVE, 16, &falhas);
  const size_t n = dupla_montar_mensagem(EXEMPLO_MATRICULA_A, EXEMPLO_MATRICULA_B, msg, sizeof msg);
  aes128_ctx ctx;
  aes128_iniciar(&ctx, chave);
  analise_cifrar_mensagem(&ctx, (const uint8_t *)msg, n, preenchida, cifrado, sizeof cifrado, &tam);
  conferir("exemplo_ficticio_pkcs7", preenchida, EXEMPLO_PREENCHIDA, 32, &falhas);
  conferir("exemplo_ficticio_cifrado", cifrado, EXEMPLO_CIFRADO, 32, &falhas);
  ecb_decifrar(&ctx, cifrado, claro, 2);
  const bool volta = pkcs7_validar(claro, 32, &tam_msg) == PKCS7_OK && tam_msg == n &&
                     memcmp(claro, msg, n) == 0;
  if (!volta) ++falhas;
  Serial.printf("[%c] AUTOTESTE %s exemplo_ficticio_decifrar_e_remover_pkcs7\n", g_papel,
                volta ? "PASS" : "FAIL");
  total += 4;
  for (int i = 0; i < 3; ++i) {
    uint8_t c[16];
    char nome[48];
    aes128_cifrar_bloco(&ctx, (const uint8_t *)ECB_CLAROS[i], c);
    snprintf(nome, sizeof nome, "ecb_pdf_bloco_%d", i + 1);
    conferir(nome, c, ECB_CIFRADOS[i], 16, &falhas);
    ++total;
  }
  Serial.printf("[%c] AUTOTESTE resumo status=%s aprovados=%d/%d\n", g_papel,
                falhas ? "FAIL" : "PASS", total - falhas, total);
  return falhas == 0;
}

// Etapa 3: chave, mensagem com PKCS#7, cifrado final, chaves de rodada 0 e 1
// e os cinco estados do bloco 1 na rodada 1, com a chave em uso.
void comum_registros_etapa3(const uint8_t *msg, size_t tamanho) {
  static uint8_t preenchida[256], cifrado[256];
  size_t tam = 0;
  Serial.printf("[%c] ETAPA3 inicio\n", g_papel);
  comum_mostrar_chave();
  log_texto("ETAPA3 MENSAGEM", msg, tamanho);
  if (analise_cifrar_mensagem(&g_ctx, msg, tamanho, preenchida, cifrado, sizeof cifrado, &tam) !=
      PKCS7_OK) {
    Serial.printf("[%c] ETAPA3 ERRO mensagem_grande_demais\n", g_papel);
    return;
  }
  Serial.printf("[%c] ETAPA3 PKCS7 tamanho_final=%u acrescentados=%u valor=0x%02x\n", g_papel,
                (unsigned)tam, (unsigned)(tam - tamanho), (unsigned)(tam - tamanho));
  log_blocos("ETAPA3 MENSAGEM_PKCS7", preenchida, tam);
  log_blocos("ETAPA3 CIFRADO", cifrado, tam);
  log_hex("ETAPA3 CHAVE_RODADA_0", g_ctx.chaves_rodada[0], 16);
  log_hex("ETAPA3 CHAVE_RODADA_1", g_ctx.chaves_rodada[1], 16);
  uint8_t estados[5][16];
  aes128_estados_rodada1(&g_ctx, preenchida, estados);
  const char *const fases[5] = {"ETAPA3 BLOCO1 (1)_AddRoundKey_inicial_chave0",
                                "ETAPA3 BLOCO1 (2)_SubBytes", "ETAPA3 BLOCO1 (3)_ShiftRows",
                                "ETAPA3 BLOCO1 (4)_MixColumns",
                                "ETAPA3 BLOCO1 (5)_AddRoundKey_chave1"};
  log_hex("ETAPA3 BLOCO1 entrada", preenchida, 16);
  for (int i = 0; i < 5; ++i) log_hex(fases[i], estados[i], 16);
  Serial.printf("[%c] ETAPA3 fim\n", g_papel);
}

// Avalanche: inverte 1 bit da chave ou da mensagem (antes do PKCS#7) e conta
// quantos bits do cifrado mudaram, no total e em cada bloco.
void comum_avalanche(bool na_chave, size_t bit, const uint8_t *msg, size_t tamanho) {
  static uint8_t m2[256], p1[256], p2[256], c1[256], c2[256];
  const size_t limite = na_chave ? 128 : 8 * tamanho;
  if (bit >= limite) {
    Serial.printf("[%c] AVALANCHE ERRO bit=%u fora de 0..%u\n", g_papel, (unsigned)bit,
                  (unsigned)(limite ? limite - 1 : 0));
    return;
  }
  aes128_ctx ctx2;
  uint8_t k2[16];
  memcpy(k2, g_chave_trabalho, 16);
  memcpy(m2, msg, tamanho);
  if (na_chave) analise_inverter_bit(k2, bit);
  else analise_inverter_bit(m2, bit);
  aes128_iniciar(&ctx2, k2);
  size_t t1 = 0, t2 = 0;
  if (analise_cifrar_mensagem(&g_ctx, msg, tamanho, p1, c1, sizeof c1, &t1) != PKCS7_OK ||
      analise_cifrar_mensagem(&ctx2, m2, tamanho, p2, c2, sizeof c2, &t2) != PKCS7_OK) {
    Serial.printf("[%c] AVALANCHE ERRO mensagem_grande_demais\n", g_papel);
    return;
  }
  Serial.printf("[%c] AVALANCHE alvo=%s bit=%u (byte %u, mascara 0x%02x)\n", g_papel,
                na_chave ? "chave" : "mensagem", (unsigned)bit, (unsigned)(bit / 8),
                (unsigned)(0x80u >> (bit % 8)));
  log_blocos("AVALANCHE CIFRADO_ORIGINAL", c1, t1);
  log_blocos("AVALANCHE CIFRADO_ALTERADO", c2, t2);
  for (size_t b = 0; b < t1 / 16; ++b) {
    const size_t d = analise_bits_diferentes(c1 + 16 * b, c2 + 16 * b, 16);
    Serial.printf("[%c] AVALANCHE bloco %u: %u/128 bits diferentes (%.2f%%)\n", g_papel,
                  (unsigned)(b + 1), (unsigned)d, 100.0 * d / 128);
  }
  const size_t d = analise_bits_diferentes(c1, c2, t1);
  Serial.printf("[%c] AVALANCHE total: %u/%u bits diferentes (%.2f%%)\n", g_papel, (unsigned)d,
                (unsigned)(8 * t1), 100.0 * d / (8 * t1));
}

// Experimento da pagina 2: chave do exemplo, tres blocos sem padding.
void comum_experimento_ecb() {
  aes128_ctx ctx;
  aes128_iniciar(&ctx, EXEMPLO_CHAVE);
  uint8_t c[3][16];
  log_hex("ECB chave_do_exemplo", EXEMPLO_CHAVE, 16);
  for (int i = 0; i < 3; ++i) {
    aes128_cifrar_bloco(&ctx, (const uint8_t *)ECB_CLAROS[i], c[i]);
    Serial.printf("[%c] ECB bloco %d claro=\"%s\" cifrado=", g_papel, i + 1, ECB_CLAROS[i]);
    for (int k = 0; k < 16; ++k) Serial.printf("%02x", c[i][k]);
    Serial.printf(" esperado_pdf=%s\n", memcmp(c[i], ECB_CIFRADOS[i], 16) ? "NAO" : "SIM");
  }
  Serial.printf("[%c] ECB bloco3_igual_bloco1=%s (blocos claros iguais -> cifrados iguais)\n",
                g_papel, memcmp(c[0], c[2], 16) ? "NAO" : "SIM");
}

void comum_iniciar(char papel, size_t buffer_rx_enlace) {
  g_papel = papel;
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.printf("[%c] BOOT firmware=%s papel=%s chip=%s revisao=%d efuse_mac=%012llx\n", papel,
                FIRMWARE_VERSAO, papel == 'A' ? "transmissor" : "receptor", ESP.getChipModel(),
                ESP.getChipRevision(), (unsigned long long)ESP.getEfuseMac());
  if (DUPLA_DADOS_FICTICIOS)
    Serial.printf("[%c] AVISO matriculas=FICTICIAS_DO_ENUNCIADO (A=%s B=%s); dados reais da "
                  "dupla PENDENTES em Codigo/src/dupla.h\n",
                  papel, DUPLA_MATRICULA_A, DUPLA_MATRICULA_B);
  g_autoteste_ok = comum_autoteste();
  if (dupla_derivar_chave(DUPLA_MATRICULA_A, DUPLA_MATRICULA_B, g_chave_dupla) != 0) {
    Serial.printf("[%c] ERRO matricula_sem_digitos; corrija Codigo/src/dupla.h\n", papel);
    g_autoteste_ok = false;
  }
  g_tamanho_mensagem_dupla = dupla_montar_mensagem(DUPLA_MATRICULA_A, DUPLA_MATRICULA_B,
                                                   g_mensagem_dupla, sizeof g_mensagem_dupla);
  if (g_tamanho_mensagem_dupla == 0) {
    Serial.printf("[%c] ERRO mensagem_da_dupla_grande_demais\n", papel);
    g_autoteste_ok = false;
  }
  definir_chave_trabalho(g_chave_dupla);
  comum_mostrar_chave();
  log_texto("MENSAGEM_DA_DUPLA", (const uint8_t *)g_mensagem_dupla, g_tamanho_mensagem_dupla);
  enlace.setRxBufferSize(buffer_rx_enlace);  // antes de begin()
  enlace.begin(115200, SERIAL_8N1, 16, 17);
  Serial.printf("[%c] UART2_PRONTA rx=16 tx=17 baud=115200 formato=8N1 quadro=AA|n|16n\n", papel);
}

bool comum_ler_linha(char *linha, size_t capacidade) {
  static char buf[MENSAGEM_MAX + 32];
  static size_t usado = 0;
  static bool excedeu = false;
  while (Serial.available()) {
    const char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      buf[usado] = '\0';
      const bool valida = !excedeu && usado > 0;
      if (excedeu) Serial.printf("[%c] CONSOLE ERRO linha_longa_demais\n", g_papel);
      usado = 0;
      excedeu = false;
      if (valida) {
        strncpy(linha, buf, capacidade - 1);
        linha[capacidade - 1] = '\0';
        return true;
      }
    } else if (usado + 1 < sizeof buf) {
      buf[usado++] = c;
    } else {
      excedeu = true;
    }
  }
  return false;
}

// Compara o inicio da linha com o comando, sem diferenciar maiusculas.
static const char *argumento(const char *linha, const char *comando) {
  const size_t n = strlen(comando);
  if (strncasecmp(linha, comando, n) != 0) return nullptr;
  if (linha[n] != '\0' && linha[n] != ' ') return nullptr;
  const char *p = linha + n;
  while (*p == ' ') ++p;
  return p;
}

static bool ler_bit(const char *arg, size_t *bit) {
  char *fim = nullptr;
  const long v = strtol(arg, &fim, 10);
  if (fim == arg || *fim != '\0' || v < 0) return false;
  *bit = (size_t)v;
  return true;
}

void comum_ajuda_comum() {
  Serial.printf("[%c] AJUDA AUTOTESTE | ETAPA3 | ECB | CHAVE | BITCHAVE <0..127> | CHAVERESET\n",
                g_papel);
  Serial.printf("[%c] AJUDA AVALANCHE CHAVE <0..127> | AVALANCHE MSG <bit da mensagem da dupla>\n",
                g_papel);
}

bool comum_tratar_comando(const char *linha) {
  const char *arg;
  size_t bit = 0;
  const uint8_t *msg = (const uint8_t *)g_mensagem_dupla;
  if ((arg = argumento(linha, "AUTOTESTE"))) {
    g_autoteste_ok = comum_autoteste() && g_tamanho_mensagem_dupla > 0;
  } else if ((arg = argumento(linha, "ETAPA3"))) {
    comum_registros_etapa3(msg, g_tamanho_mensagem_dupla);
  } else if ((arg = argumento(linha, "ECB"))) {
    comum_experimento_ecb();
  } else if ((arg = argumento(linha, "CHAVERESET"))) {
    definir_chave_trabalho(g_chave_dupla);
    comum_mostrar_chave();
  } else if ((arg = argumento(linha, "CHAVE"))) {
    comum_mostrar_chave();
  } else if ((arg = argumento(linha, "BITCHAVE"))) {
    if (!ler_bit(arg, &bit) || bit >= 128) {
      Serial.printf("[%c] CONSOLE ERRO uso: BITCHAVE <0..127>\n", g_papel);
      return true;
    }
    uint8_t k[16];
    memcpy(k, g_chave_trabalho, 16);
    analise_inverter_bit(k, bit);
    definir_chave_trabalho(k);
    Serial.printf("[%c] BITCHAVE bit=%u invertido na chave desta placa\n", g_papel, (unsigned)bit);
    comum_mostrar_chave();
  } else if ((arg = argumento(linha, "AVALANCHE"))) {
    const char *sub;
    if ((sub = argumento(arg, "CHAVE")) && ler_bit(sub, &bit))
      comum_avalanche(true, bit, msg, g_tamanho_mensagem_dupla);
    else if ((sub = argumento(arg, "MSG")) && ler_bit(sub, &bit))
      comum_avalanche(false, bit, msg, g_tamanho_mensagem_dupla);
    else
      Serial.printf("[%c] CONSOLE ERRO uso: AVALANCHE CHAVE <bit> | AVALANCHE MSG <bit>\n", g_papel);
  } else {
    return false;
  }
  return true;
}
