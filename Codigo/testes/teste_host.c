// Testes no computador do mesmo codigo C usado nas placas (Codigo/src).
// Execucao no PC: NAO substitui a validacao nas duas placas reais.
// Compilar e executar: Codigo/testes/executar_testes.sh

#include <stdio.h>
#include <string.h>

#include "aes128.h"
#include "analise.h"
#include "dupla.h"
#include "pkcs7_ecb.h"
#include "quadro.h"
#include "vetores_pdf.h"

static int falhas = 0, verificacoes = 0;

static void hex(const uint8_t *b, size_t n) {
  for (size_t i = 0; i < n; ++i) printf("%02x", b[i]);
}

static void conferir_bytes(const char *nome, const uint8_t *obtido, const uint8_t *esperado,
                           size_t n) {
  ++verificacoes;
  const int ok = memcmp(obtido, esperado, n) == 0;
  if (!ok) ++falhas;
  printf("%s %s obtido=", ok ? "PASS" : "FAIL", nome);
  hex(obtido, n);
  if (!ok) {
    printf(" esperado=");
    hex(esperado, n);
  }
  printf("\n");
}

static void conferir(const char *nome, int condicao) {
  ++verificacoes;
  if (!condicao) ++falhas;
  printf("%s %s\n", condicao ? "PASS" : "FAIL", nome);
}

static void de_hex(const char *s, uint8_t *out) {
  for (size_t i = 0; s[2 * i]; ++i) {
    unsigned v;
    sscanf(s + 2 * i, "%2x", &v);
    out[i] = (uint8_t)v;
  }
}

static void teste_sbox(void) {
  aes_gerar_tabelas();
  // Valores conhecidos da tabela da S-box (FIPS-197 figura 7).
  conferir("sbox[00]=63", AES_SBOX[0x00] == 0x63);
  conferir("sbox[01]=7c", AES_SBOX[0x01] == 0x7c);
  conferir("sbox[53]=ed", AES_SBOX[0x53] == 0xed);
  conferir("sbox[ff]=16", AES_SBOX[0xff] == 0x16);
  int permutacao = 1;
  for (int x = 0; x < 256; ++x)
    if (AES_SBOX_INV[AES_SBOX[x]] != x) permutacao = 0;
  conferir("sbox inversa desfaz a sbox nos 256 valores", permutacao);
  conferir("xtime(57)=ae (FIPS-197 4.2.1)", aes_xtime(0x57) == 0xae);
  conferir("57*83=c1 (FIPS-197 4.2)", aes_gmul(0x57, 0x83) == 0xc1);
}

static void teste_fips(void) {
  for (int i = 0; i < 2; ++i) {
    const vetor_bloco *v = &VETORES_FIPS[i];
    aes128_ctx ctx;
    aes128_iniciar(&ctx, v->chave);
    uint8_t c[16], p[16];
    char nome[80];
    aes128_cifrar_bloco(&ctx, v->claro, c);
    snprintf(nome, sizeof nome, "%s cifrar", v->nome);
    conferir_bytes(nome, c, v->cifrado, 16);
    aes128_decifrar_bloco(&ctx, v->cifrado, p);
    snprintf(nome, sizeof nome, "%s decifrar", v->nome);
    conferir_bytes(nome, p, v->claro, 16);
  }
}

// Chaves de rodada e estados da rodada 1 publicados no FIPS-197 (Apendices
// A.1 e B) para a chave 2b7e... e o texto 3243...; conferem o registro da Etapa 3.
static void teste_rodada1_fips(void) {
  aes128_ctx ctx;
  aes128_iniciar(&ctx, VETORES_FIPS[0].chave);
  uint8_t esperado[16];
  conferir_bytes("FIPS A.1 chave de rodada 0", ctx.chaves_rodada[0], VETORES_FIPS[0].chave, 16);
  de_hex("a0fafe1788542cb123a339392a6c7605", esperado);
  conferir_bytes("FIPS A.1 chave de rodada 1", ctx.chaves_rodada[1], esperado, 16);
  de_hex("d014f9a8c9ee2589e13f0cc8b6630ca6", esperado);
  conferir_bytes("FIPS A.1 chave de rodada 10", ctx.chaves_rodada[10], esperado, 16);

  uint8_t estados[5][16];
  aes128_estados_rodada1(&ctx, VETORES_FIPS[0].claro, estados);
  const char *const esperados[5] = {
    "193de3bea0f4e22b9ac68d2ae9f84808",  // apos AddRoundKey inicial
    "d42711aee0bf98f1b8b45de51e415230",  // apos SubBytes
    "d4bf5d30e0b452aeb84111f11e2798e5",  // apos ShiftRows
    "046681e5e0cb199a48f8d37a2806264c",  // apos MixColumns
    "a49c7ff2689f352b6b5bea43026a5049",  // apos AddRoundKey(chave 1)
  };
  const char *const fases[5] = {"(1) AddRoundKey inicial", "(2) SubBytes", "(3) ShiftRows",
                                "(4) MixColumns", "(5) AddRoundKey chave 1"};
  for (int i = 0; i < 5; ++i) {
    char nome[80];
    de_hex(esperados[i], esperado);
    snprintf(nome, sizeof nome, "FIPS B rodada 1 %s", fases[i]);
    conferir_bytes(nome, estados[i], esperado, 16);
  }
  // Cada inversa desfaz sua transformacao.
  uint8_t s[16];
  memcpy(s, estados[3], 16);
  aes_inv_mix_columns(s);
  conferir_bytes("InvMixColumns desfaz MixColumns", s, estados[2], 16);
  aes_inv_shift_rows(s);
  conferir_bytes("InvShiftRows desfaz ShiftRows", s, estados[1], 16);
  aes_inv_sub_bytes(s);
  conferir_bytes("InvSubBytes desfaz SubBytes", s, estados[0], 16);
}

static void teste_exemplo_pdf(void) {
  uint8_t chave[16];
  conferir("exemplo: derivar chave",
           dupla_derivar_chave(EXEMPLO_MATRICULA_A, EXEMPLO_MATRICULA_B, chave) == 0);
  conferir_bytes("exemplo: chave ASCII", chave, EXEMPLO_CHAVE, 16);
  char msg[64];
  const size_t n = dupla_montar_mensagem(EXEMPLO_MATRICULA_A, EXEMPLO_MATRICULA_B, msg, sizeof msg);
  conferir("exemplo: mensagem A:20231001;B:20231002 (21 bytes)",
           n == 21 && strcmp(msg, EXEMPLO_MENSAGEM) == 0);
  aes128_ctx ctx;
  aes128_iniciar(&ctx, chave);
  uint8_t preenchida[32], cifrado[32];
  size_t tam = 0;
  conferir("exemplo: cifrar mensagem",
           analise_cifrar_mensagem(&ctx, (const uint8_t *)msg, n, preenchida, cifrado,
                                   sizeof cifrado, &tam) == PKCS7_OK && tam == 32);
  conferir_bytes("exemplo: mensagem com PKCS#7", preenchida, EXEMPLO_PREENCHIDA, 32);
  conferir_bytes("exemplo: cifrado ECB", cifrado, EXEMPLO_CIFRADO, 32);

  uint8_t q[QUADRO_MAX_BYTES];
  const size_t tq = quadro_montar(cifrado, 2, q, sizeof q);
  uint8_t esperado_q[34] = {0xAA, 0x02};
  memcpy(esperado_q + 2, EXEMPLO_CIFRADO, 32);
  conferir("exemplo: quadro tem 34 bytes", tq == 34);
  conferir_bytes("exemplo: quadro AA|02|blocos", q, esperado_q, 34);

  uint8_t claro[32];
  size_t tam_msg = 0;
  ecb_decifrar(&ctx, cifrado, claro, 2);
  conferir("exemplo: padding valido na decifragem",
           pkcs7_validar(claro, 32, &tam_msg) == PKCS7_OK && tam_msg == 21);
  conferir("exemplo: mensagem recuperada identica", memcmp(claro, msg, 21) == 0);
}

static void teste_ecb_pdf(void) {
  aes128_ctx ctx;
  aes128_iniciar(&ctx, EXEMPLO_CHAVE);
  for (int i = 0; i < 3; ++i) {
    uint8_t c[16];
    char nome[80];
    aes128_cifrar_bloco(&ctx, (const uint8_t *)ECB_CLAROS[i], c);
    snprintf(nome, sizeof nome, "ECB bloco %d \"%s\"", i + 1, ECB_CLAROS[i]);
    conferir_bytes(nome, c, ECB_CIFRADOS[i], 16);
  }
}

static void teste_chave_regras(void) {
  uint8_t k[16];
  conferir("chave: matricula curta completa com 0 a esquerda",
           dupla_derivar_chave("1234", "567", k) == 0 &&
               memcmp(k, "0000123400000567", 16) == 0);
  conferir("chave: usa os 8 ultimos digitos",
           dupla_derivar_chave("202312345678", "9876543210", k) == 0 &&
               memcmp(k, "1234567876543210", 16) == 0);
  conferir("chave: ignora caracteres que nao sao digitos",
           dupla_derivar_chave("2023.100-1", "x20231002", k) == 0 &&
               memcmp(k, "2023100120231002", 16) == 0);
  conferir("chave: matricula sem digitos e rejeitada", dupla_derivar_chave("abc", "1", k) != 0);
}

// Dados reais configurados em dupla.h. O cifrado esperado foi calculado com o
// OpenSSL 3.6.5 apenas como referencia externa de conferencia.
static void teste_dados_da_dupla(void) {
  uint8_t chave[16];
  conferir("dupla: dados reais configurados (DUPLA_DADOS_FICTICIOS=0)", DUPLA_DADOS_FICTICIOS == 0);
  conferir("dupla: chave = ultimos 8 de A + ultimos 8 de B",
           dupla_derivar_chave(DUPLA_MATRICULA_A, DUPLA_MATRICULA_B, chave) == 0 &&
               memcmp(chave, "7844001327844251", 16) == 0);
  char msg[64];
  const size_t n = dupla_montar_mensagem(DUPLA_MATRICULA_A, DUPLA_MATRICULA_B, msg, sizeof msg);
  conferir("dupla: mensagem A:2023178440013;B:202127844251 (30 bytes)",
           n == 30 && strcmp(msg, "A:2023178440013;B:202127844251") == 0);
  aes128_ctx ctx;
  aes128_iniciar(&ctx, chave);
  uint8_t preenchida[32], cifrado[32], esperado[32];
  size_t tam = 0;
  analise_cifrar_mensagem(&ctx, (const uint8_t *)msg, n, preenchida, cifrado, sizeof cifrado, &tam);
  conferir("dupla: PKCS#7 acrescenta 2 bytes 0x02",
           tam == 32 && preenchida[30] == 0x02 && preenchida[31] == 0x02);
  de_hex("2e3e460ec295383cec6a018765ff053130b51beef909b9d823300de92a5e9fb3", esperado);
  conferir_bytes("dupla: cifrado igual a referencia OpenSSL", cifrado, esperado, 32);
}

static void teste_pkcs7(void) {
  uint8_t buf[64], m[48];
  size_t t = 0, tm = 0;
  for (int i = 0; i < 48; ++i) m[i] = (uint8_t)i;
  const size_t tamanhos[] = {0, 1, 15, 16, 17, 31, 32, 33};
  for (size_t k = 0; k < sizeof tamanhos / sizeof tamanhos[0]; ++k) {
    const size_t n = tamanhos[k];
    char nome[80];
    const int ok = pkcs7_preencher(m, n, buf, sizeof buf, &t) == PKCS7_OK &&
                   t == (n / 16 + 1) * 16 && buf[t - 1] == t - n &&
                   pkcs7_validar(buf, t, &tm) == PKCS7_OK && tm == n && memcmp(buf, m, n) == 0;
    snprintf(nome, sizeof nome, "PKCS#7 ida e volta com %zu bytes", n);
    conferir(nome, ok);
  }
  memset(buf, 0x10, 16);
  conferir("PKCS#7: bloco inteiro de 0x10 e valido", pkcs7_validar(buf, 16, &tm) == PKCS7_OK && tm == 0);
  buf[15] = 0x00;
  conferir("PKCS#7: rejeita valor 0x00", pkcs7_validar(buf, 16, &tm) == PKCS7_ERRO_VALOR);
  buf[15] = 0x11;
  conferir("PKCS#7: rejeita valor > 16", pkcs7_validar(buf, 16, &tm) == PKCS7_ERRO_VALOR);
  memset(buf, 0x03, 16);
  buf[13] = 0x04;
  conferir("PKCS#7: rejeita bytes inconsistentes", pkcs7_validar(buf, 16, &tm) == PKCS7_ERRO_BYTES);
  conferir("PKCS#7: rejeita tamanho nao multiplo de 16",
           pkcs7_validar(buf, 15, &tm) == PKCS7_ERRO_TAMANHO);
  conferir("PKCS#7: rejeita tamanho zero", pkcs7_validar(buf, 0, &tm) == PKCS7_ERRO_TAMANHO);
  conferir("PKCS#7: recusa saida sem capacidade",
           pkcs7_preencher(m, 16, buf, 16, &t) == PKCS7_ERRO_CAPACIDADE);
}

static quadro_rx rx;  // grande demais para a pilha em alguns sistemas

static quadro_rx_evento alimentar(const uint8_t *b, size_t n, uint32_t *t, uint32_t passo) {
  quadro_rx_evento ultimo = QRX_NADA;
  for (size_t i = 0; i < n; ++i) {
    const quadro_rx_evento e = quadro_rx_byte(&rx, b[i], *t);
    if (e != QRX_NADA) ultimo = e;
    *t += passo;
  }
  return ultimo;
}

static void teste_receptor(void) {
  uint8_t q[34] = {0xAA, 0x02};
  memcpy(q + 2, EXEMPLO_CIFRADO, 32);
  uint32_t t = 1000;

  quadro_rx_iniciar(&rx);
  const uint8_t lixo[] = {0x00, 0x13, 0xff};
  alimentar(lixo, sizeof lixo, &t, 1);
  const quadro_rx_evento e = alimentar(q, sizeof q, &t, 1);
  conferir("receptor: sincroniza no 0xAA depois de lixo",
           e == QRX_QUADRO_COMPLETO && rx.n == 2 && rx.descartados == 3 &&
               memcmp(rx.dados, EXEMPLO_CIFRADO, 32) == 0);

  quadro_rx_iniciar(&rx);
  alimentar(q, 20, &t, 1);
  conferir("receptor: quadro incompleto expira",
           quadro_rx_verificar_timeout(&rx, t + QUADRO_TIMEOUT_ENTRE_BYTES_MS + 1) == QRX_ERRO_TIMEOUT &&
               rx.estado == QRX_AGUARDA_MARCADOR);
  t += QUADRO_TIMEOUT_ENTRE_BYTES_MS + 2;
  conferir("receptor: quadro seguinte completo apos timeout",
           alimentar(q, sizeof q, &t, 1) == QRX_QUADRO_COMPLETO);

  quadro_rx_iniciar(&rx);
  const uint8_t n_zero[] = {0xAA, 0x00};
  conferir("receptor: n=0 rejeitado", alimentar(n_zero, 2, &t, 1) == QRX_ERRO_N_ZERO);

  quadro_rx_iniciar(&rx);
  alimentar(q, 10, &t, 1);
  t += QUADRO_TIMEOUT_ENTRE_BYTES_MS + 5;  // pausa longa sem chamar verificar_timeout
  conferir("receptor: byte atrasado abandona o quadro", quadro_rx_byte(&rx, 0x55, t) == QRX_ERRO_TIMEOUT);
  conferir("receptor: e depois aceita quadro completo",
           alimentar(q, sizeof q, &t, 1) == QRX_QUADRO_COMPLETO);

  quadro_rx_iniciar(&rx);
  alimentar(q, 2, &t, 0);
  uint32_t lento = t;
  for (size_t i = 2; i < sizeof q; ++i) {
    quadro_rx_byte(&rx, q[i], lento);
    lento += 60;  // abaixo do limite entre bytes, mas soma > limite total
  }
  conferir("receptor: limite total de tempo do quadro",
           rx.estado == QRX_AGUARDA_MARCADOR && lento - t > QUADRO_TIMEOUT_TOTAL_MS);

  uint8_t grande[QUADRO_MAX_BYTES];
  static uint8_t cif[QUADRO_MAX_DADOS];
  memset(cif, 0x5a, sizeof cif);
  conferir("quadro: 255 blocos montado", quadro_montar(cif, 255, grande, sizeof grande) == 4082);
  conferir("quadro: 0 blocos recusado", quadro_montar(cif, 0, grande, sizeof grande) == 0);
  conferir("quadro: 256 blocos recusado", quadro_montar(cif, 256, grande, sizeof grande) == 0);
}

static void teste_avalanche(void) {
  uint8_t a[2] = {0x00, 0x00};
  analise_inverter_bit(a, 0);
  analise_inverter_bit(a, 15);
  conferir("bit 0 = 0x80 do byte 0; bit 15 = 0x01 do byte 1", a[0] == 0x80 && a[1] == 0x01);
  const uint8_t z[2] = {0, 0};
  conferir("contagem de bits diferentes", analise_bits_diferentes(a, z, 2) == 2);

  // Mesmo bloco, chave com 1 bit invertido: imprime a contagem (nao e PASS/FAIL).
  aes128_ctx c1, c2;
  uint8_t k2[16], o1[16], o2[16];
  memcpy(k2, VETORES_FIPS[0].chave, 16);
  analise_inverter_bit(k2, 0);
  aes128_iniciar(&c1, VETORES_FIPS[0].chave);
  aes128_iniciar(&c2, k2);
  aes128_cifrar_bloco(&c1, VETORES_FIPS[0].claro, o1);
  aes128_cifrar_bloco(&c2, VETORES_FIPS[0].claro, o2);
  printf("INFO avalanche FIPS B, bit 0 da chave: %zu/128 bits do cifrado mudaram\n",
         analise_bits_diferentes(o1, o2, 16));
}

int main(void) {
  printf("Testes no computador (nao e execucao nas placas)\n");
  teste_sbox();
  teste_fips();
  teste_rodada1_fips();
  teste_exemplo_pdf();
  teste_ecb_pdf();
  teste_chave_regras();
  teste_dados_da_dupla();
  teste_pkcs7();
  teste_receptor();
  teste_avalanche();
  printf("RESUMO verificacoes=%d falhas=%d status=%s\n", verificacoes, falhas,
         falhas ? "FAIL" : "PASS");
  return falhas ? 1 : 0;
}
