# Slides — conteúdo e notas de fala

Gerados de código e evidências existentes; sem nova execução nas placas.

## 01 — AES-128 entre dois ESP32

TRABALHO PRÁTICO

AES-128 entre dois ESP32

Criptografia, comunicação UART e recuperação da mensagem.

Uma mensagem.
Duas placas.
O mesmo texto no final.

ESP32

USB

A envia

DADOS REAIS DA DUPLA

Augusto · 2023178440013
Gustavo · 202127844251

AES-128 · AUGUSTO + GUSTAVO

01 / 27

**Notas de fala:** Apresentar Augusto (A) e Gustavo (B). O AES usa a mesma chave para cifrar e decifrar. Os resultados citados são de logs existentes de 19/10/2026; esta geração de slides não executou hardware.

**Fontes locais:** AES_Trabalho.pdf; Codigo/evidencias/aes-dupla-2026-10-19.json

## 02 — O que queremos mostrar?

ENTENDA O SISTEMA

O que queremos mostrar?

A mensagem muda de aparência durante o transporte e volta ao texto original.

TEXTO ORIGINAL

A:2023…

ESP32 A

Cifra com a chave

UART

2e 3e 46 0e …

ESP32 B

Decifra com a chave

O teste final compara o texto original com o texto recuperado, byte por byte.

AES-128 · AUGUSTO + GUSTAVO

02 / 27

**Notas de fala:** 

**Fontes locais:** AES_Trabalho.pdf, páginas 1–2

## 03 — Dois caminhos: UART para dados, USB para observar

MONTAGEM FÍSICA

Dois caminhos: UART para dados, USB para observar

TX é a saída de dados. RX é a entrada. GND dá uma referência elétrica comum.

ESP32

USB

A · transmissor

ESP32

USB

B · receptor

GPIO17 / TX

GPIO16 / RX

GPIO16 / RX

GPIO17 / TX

GND comum

UART2: 115200 bps, 8N1. USB de cada placa: um monitor serial.

AES-128 · AUGUSTO + GUSTAVO

03 / 27

**Notas de fala:** Os módulos são ESP32-WROOM-32. A: USB location 3-3. B: 3-1. Portas ttyUSB podem mudar. A transmissão AES atual é A→B; o fio de retorno existe, mas A descarta os dados de entrada. Não afirmar AES bidirecional com o firmware atual.

**Fontes locais:** Codigo/src/comum.cpp; Codigo/evidencias/aes-dupla-2026-10-19.json

## 04 — Um texto também é uma sequência de números

ENTENDA O SISTEMA

Um texto também é uma sequência de números

Byte = grupo de 8 bits. Hexadecimal = uma forma curta de escrever esses números.

CARACTERE

O símbolo que lemos:

7

BYTE EM HEX

O código ASCII de “7”:

0x37

=

BITS

O mesmo byte:

00110111

=

AES-128 usa uma chave de 128 bits e trabalha com blocos de 16 bytes.

AES-128 · AUGUSTO + GUSTAVO

04 / 27

**Notas de fala:** 

**Fontes locais:** AES_Trabalho.pdf, páginas 1–2

## 05 — A chave nasce das matrículas

DADOS DA DUPLA

A chave nasce das matrículas

Pegamos os últimos 8 dígitos de A e os últimos 8 de B. Depois juntamos A + B.

A · Augusto

2023178440013 → 78440013

B · Gustavo

202127844251 → 27844251

CHAVE ASCII · 16 BYTES

7844001327844251

Regra do trabalho: dados previsíveis. Em um produto, a chave deve ser secreta e imprevisível.

AES-128 · AUGUSTO + GUSTAVO

05 / 27

**Notas de fala:** Se houver menos de oito dígitos, completar com zero à esquerda. Cada caractere vira um byte ASCII; não converter a sequência para um número inteiro. Regra acadêmica: matrículas são previsíveis e não representam uma chave secreta adequada para um produto.

**Fontes locais:** Codigo/src/dupla.c; Codigo/src/dupla.h; evidência aes-dupla-2026-10-19.json

## 06 — O AES precisa de blocos completos

PREPARAR A MENSAGEM

O AES precisa de blocos completos

Nossa mensagem tem 30 bytes. O PKCS#7 acrescenta 02 02 para chegar a 32.

A:2023178440013;B:202127844251

BLOCO 1

41

3a

32

30

32

33

31

37

38

34

34

30

30

31

33

3b

BLOCO 2

42

3a

32

30

32

31

32

37

38

34

34

32

35

31

02

02

Se a mensagem já for múltipla de 16, entram 16 bytes de valor 0x10.

AES-128 · AUGUSTO + GUSTAVO

06 / 27

**Notas de fala:** A mensagem real é A:2023178440013;B:202127844251. São dois blocos de 16 bytes. PKCS#7 sempre acrescenta entre 1 e 16 bytes; se já couber exatamente, acrescenta um bloco completo de 0x10. Não é texto “02”: são bytes de valor 2.

**Fontes locais:** Codigo/src/pkcs7_ecb.c; evidência aes-dupla-2026-10-19.json

## 07 — No código: completar e cifrar

CÓDIGO REAL

No código: completar e cifrar

A preparação do texto e a cifra são etapas diferentes.

pkcs7_ecb.c · dentro de pkcs7_preencher()

  const uint8_t falta = (uint8_t)(total - tamanho);  // 1..16
  if (tamanho) memmove(saida, msg, tamanho);
  memset(saida + tamanho, falta, falta);

pkcs7_ecb.c · dentro de ecb_cifrar()

  for (size_t b = 0; b < blocos; ++b)
    aes128_cifrar_bloco(ctx, in + 16 * b, out + 16 * b);

Cada chamada trabalha com 16 bytes. A mesma chave é usada nos dois lados.

AES-128 · AUGUSTO + GUSTAVO

07 / 27

**Notas de fala:** Os trechos foram extraídos literalmente dos arquivos de origem. Explicar que falta é o total de bytes de preenchimento e também o valor de cada byte acrescentado. No modo ECB, a mesma função cifra cada bloco separadamente.

**Fontes locais:** Codigo/src/pkcs7_ecb.c; Codigo/src/analise.c

## 08 — O AES transforma um bloco em 10 rodadas

POR DENTRO DO AES

O AES transforma um bloco em 10 rodadas

A cada rodada, substituímos, deslocamos, misturamos e combinamos com a chave.

ANTES DAS RODADAS

AddRoundKey com a chave 0

SubBytes

Troca cada byte

ShiftRows

Desloca as linhas

MixColumns

Mistura as colunas

AddRoundKey

Combina com a chave

Rodadas 1 a 9: as quatro etapas. Rodada 10: sem MixColumns.

AES-128 · AUGUSTO + GUSTAVO

08 / 27

**Notas de fala:** O estado tem 16 bytes, organizados em uma matriz 4×4 preenchida por colunas. Há AddRoundKey inicial, rodadas 1–9 completas e rodada 10 sem MixColumns. A chave expandida fornece chaves 0–10.

**Fontes locais:** Codigo/src/aes128.c · aes128_cifrar_bloco()

## 09 — SubBytes: trocar cada byte por outro

TRANSFORMAÇÃO 1

SubBytes: trocar cada byte por outro

A tabela S-box define a substituição. Isso dificulta relações simples entre entrada e saída.

76

S-box

38

Exemplo do
primeiro bloco
da dupla.

aes128.c · código real

void aes_sub_bytes(uint8_t s[16]) {
  for (int i = 0; i < 16; ++i) s[i] = AES_SBOX[s[i]];
}

AES-128 · AUGUSTO + GUSTAVO

09 / 27

**Notas de fala:** Exemplo real: estado após AddRoundKey inicial 0x76 vira 0x38. A S-box é construída pelo inverso multiplicativo em GF(2⁸), com 0 mapeado para 0, seguido de transformação afim e constante 0x63. A implementação calcula a tabela, não usa biblioteca criptográfica.

**Fontes locais:** Codigo/src/aes128.c · aes_sub_bytes() e aes_gerar_tabelas()

## 10 — ShiftRows: cada linha anda um pouco

TRANSFORMAÇÃO 2

ShiftRows: cada linha anda um pouco

Linha 0 fica. As outras giram 1, 2 e 3 posições para a esquerda.

38

77

67

f2

77

7b

7b

7b

6f

63

fe

6f

f2

f2

f2

67

ANTES

38

77

67

f2

7b

7b

7b

77

fe

6f

6f

63

67

f2

f2

f2

DEPOIS

Linha 0

fica no lugar

Linha 1

1 posição à esquerda

Linha 2

2 posições à esquerda

Linha 3

3 posições à esquerda

AES-128 · AUGUSTO + GUSTAVO

10 / 27

**Notas de fala:** Mostrar que os valores não mudam nesta etapa: mudam de posição. A entrada e saída são estados reais da rodada 1. A leitura do hexadecimal nos registros é por colunas, não por linhas.

**Fontes locais:** Codigo/src/aes128.c · aes_shift_rows(); evidência aes-dupla-2026-10-19.json

## 11 — MixColumns: uma coluna se mistura por dentro

TRANSFORMAÇÃO 3

MixColumns: uma coluna se mistura por dentro

Cada byte de saída depende dos quatro bytes daquela coluna.

38

7b

fe

67

Misturar com
regras do AES

64

b0

0d

03

aes128.c · multiplicação por 02

uint8_t aes_xtime(uint8_t a) {
  return (uint8_t)((a << 1) ^ ((a & 0x80) ? 0x1b : 0x00));
}

AES-128 · AUGUSTO + GUSTAVO

11 / 27

**Notas de fala:** A primeira coluna real antes desta transformação é 38 7b fe 67; depois é 64 b0 0d 03. Os cálculos são em GF(2⁸), usando XOR em vez de soma comum. 02·a = xtime(a); 03·a = xtime(a) XOR a. O polinômio é 0x11b.

**Fontes locais:** Codigo/src/aes128.c · aes_mix_columns() e aes_xtime()

## 12 — AddRoundKey: a chave entra na transformação

TRANSFORMAÇÃO 4

AddRoundKey: a chave entra na transformação

Usamos XOR: bits iguais dão 0; bits diferentes dão 1.

41

XOR

37

=

76

byte da mensagem

byte da chave

novo byte

aes128.c · código real

void aes_add_round_key(uint8_t s[16], const uint8_t chave_rodada[16]) {
  for (int i = 0; i < 16; ++i) s[i] ^= chave_rodada[i];
}

AES-128 · AUGUSTO + GUSTAVO

12 / 27

**Notas de fala:** Primeiro byte da mensagem 0x41 XOR primeiro byte da chave 0x37 resulta 0x76. XOR é reversível: aplicar novamente a mesma chave recupera o valor anterior. Não confundir esta etapa isolada com todo o algoritmo AES.

**Fontes locais:** Codigo/src/aes128.c · aes_add_round_key(); evidência aes-dupla-2026-10-19.json

## 13 — Uma chave inicial gera as chaves das rodadas

EXPANSÃO DE CHAVE

Uma chave inicial gera as chaves das rodadas

AES-128 usa 11 chaves de 16 bytes: a inicial e uma para cada rodada.

RotWord
Gira bytes

SubWord
Aplica S-box

Rcon
Inclui constante

XOR
Deriva palavras

CHAVE DA RODADA 0

37383434303031333237383434323531

CHAVE DA RODADA 1

15aef32c259ec21f17a9fa2b239bcf1a

AES-128 · AUGUSTO + GUSTAVO

13 / 27

**Notas de fala:** A expansão produz 44 palavras de 4 bytes. RotWord gira a palavra; SubWord aplica S-box; Rcon adiciona uma constante que muda por rodada. As chaves não são geradas aleatoriamente: são derivadas da chave inicial.

**Fontes locais:** Codigo/src/aes128.c · aes128_iniciar(); evidência aes-dupla-2026-10-19.json

## 14 — O ciclo completo aparece neste trecho

CÓDIGO REAL

O ciclo completo aparece neste trecho

A última rodada pula MixColumns, como exige o AES.

aes128.c · dentro de aes128_cifrar_bloco()

  aes_add_round_key(s, ctx->chaves_rodada[0]);
  for (int r = 1; r <= AES128_RODADAS; ++r) {
    aes_sub_bytes(s);
    aes_shift_rows(s);
    if (r != AES128_RODADAS) aes_mix_columns(s);
    aes_add_round_key(s, ctx->chaves_rodada[r]);
  }

Para voltar ao texto: etapas inversas e chaves usadas da rodada 10 até a 0.

AES-128 · AUGUSTO + GUSTAVO

14 / 27

**Notas de fala:** Trecho literal da função de cifragem. Os buffers in/out são copiados antes/depois deste trecho. AES128_RODADAS vale 10. Para decifrar, o código usa transformações inversas e as chaves em ordem contrária.

**Fontes locais:** Codigo/src/aes128.c · aes128_cifrar_bloco()

## 15 — Como B sabe onde a mensagem começa e termina?

PROTOCOLO UART

Como B sabe onde a mensagem começa e termina?

O quadro informa o início e a quantidade de blocos cifrados.

AA

início
1 byte

02

2 blocos
1 byte

ciphertext 1

dados cifrados
16 bytes

ciphertext 2

dados cifrados
16 bytes

transmissor.cpp · bytes brutos na UART2

const size_t escritos = enlace.write(quadro, tamanho_quadro);
enlace.flush();

AES-128 · AUGUSTO + GUSTAVO

15 / 27

**Notas de fala:** AA é byte fixo de início, não a string de dois caracteres A e A. n=02 significa dois blocos. A carga cifrada pode conter 0xAA: depois de ler n o receptor segue o comprimento, não procura outro marcador no meio. Cabeçalho não é autenticado.

**Fontes locais:** Codigo/src/quadro.c; Codigo/src/transmissor.cpp; AES_Trabalho.pdf, p. 1

## 16 — Na placa B, o caminho acontece ao contrário

CÓDIGO REAL

Na placa B, o caminho acontece ao contrário

Receber → decifrar → conferir o preenchimento → recuperar a mensagem.

receptor.cpp · dentro de processar_quadro()

  ecb_decifrar(&g_ctx, rx.dados, claro, rx.n);
  log_blocos("DECIFRADO_COM_PKCS7", claro, tamanho);
  size_t tamanho_msg = 0;
  const pkcs7_resultado r = pkcs7_validar(claro, tamanho, &tamanho_msg);

PADDING VÁLIDO

Remove o preenchimento e mostra o texto.

PADDING INVÁLIDO

Descarta a mensagem e registra o motivo.

AES-128 · AUGUSTO + GUSTAVO

16 / 27

**Notas de fala:** O receptor lê exatamente 16×n bytes, com timeout entre bytes de 100 ms e total de 1 s. B decifra os blocos antes de validar/remover PKCS#7. Neste trecho tamanho já é conhecido. Em caso de padding inválido, o código descarta a mensagem.

**Fontes locais:** Codigo/src/receptor.cpp · processar_quadro(); Codigo/src/quadro.c

## 17 — O que a turma verá nos dois monitores

DEMONSTRAÇÃO

O que a turma verá nos dois monitores

Trechos reais dos logs de 19/10/2026, com as matrículas da dupla.

MONITOR A · transmite

[A] MENSAGEM_ORIGINAL tamanho=30
A:2023178440013;B:202127844251

[A] CIFRADO_ENVIADO bloco 1:
2e3e460ec295383cec6a018765ff0531
[A] CIFRADO_ENVIADO bloco 2:
30b51beef909b9d823300de92a5e9fb3

MONITOR B · recebe

[B] CIFRADO_RECEBIDO bloco 1:
2e3e460ec295383cec6a018765ff0531
[B] CIFRADO_RECEBIDO bloco 2:
30b51beef909b9d823300de92a5e9fb3

[B] MENSAGEM_RECUPERADA tamanho=30
A:2023178440013;B:202127844251

Registro real de B: COMPARACAO igual_a_mensagem_da_dupla=SIM

AES-128 · AUGUSTO + GUSTAVO

17 / 27

**Notas de fala:** Os painéis mostram linhas reais selecionadas, com quebras visuais para caber. Não são capturas de tela nem uma nova transmissão. Na aula abrir ambos os monitores antes de enviar. Comparar os dois blocos e a mensagem inteira.

**Fontes locais:** Codigo/evidencias/aes-dupla-a-2026-10-19.log; aes-dupla-b-2026-10-19.log

## 18 — Demonstração ao vivo: três passos

ROTEIRO PARA A SALA

Demonstração ao vivo: três passos

Usar os dois monitores USB a 115200 bps, com envio de linha LF.

1 · PREPARAR

A e B: CHAVERESET
A: MSGRESET
B: aguardar PRONTO

2 · ENVIAR

A: ENVIAR

ou apertar BOOT
após o boot normal.

3 · CONFERIR

Mesmos blocos cifrados.

Texto recuperado igual ao
original.

Alteração na arguição: MSG <novo texto>. Para voltar: MSGRESET e ENVIAR.

AES-128 · AUGUSTO + GUSTAVO

18 / 27

**Notas de fala:** Procedimento para executar na aula, não declaração de um novo ensaio. Antes: conferir firmware, identidade física, GND e cruzamento TX/RX. Restaurar as chaves em A e B e mensagem em A. B deve estar pronto antes de ENVIAR. A comparação automática de B é contra a mensagem padrão da dupla: com MSG nova, NAO é esperado e não indica por si só falha na decifragem.

**Fontes locais:** Codigo/README.md; Codigo/src/transmissor.cpp; Codigo/src/comum.cpp

## 19 — Como sabemos que o resultado está correto?

EVIDÊNCIAS REAIS

Como sabemos que o resultado está correto?

Conferimos valores conhecidos antes de usar a chave da dupla.

71 / 71

Verificações no computador

11 / 11

Autoteste na placa A

11 / 11

Autoteste na placa B

FIPS-197 C.1 · ciphertext esperado e conferido

69c4e0d86a7b0430d8cdb78070b4c55a

Mil pacotes AES, latência AES e envio AES B→A: sem validação documentada.

AES-128 · AUGUSTO + GUSTAVO

19 / 27

**Notas de fala:** 71 verificações no computador não equivalem a 71 ensaios físicos. As placas executaram 11 itens de autoteste cada uma. Os vetores são FIPS-197 2001 B e C.1 conforme enunciado. As capturas AES existentes são A→B; não há evidência de teste AES bidirecional ou sequência AES de mil pacotes.

**Fontes locais:** Codigo/evidencias/testes-host-dupla-2026-10-19.log; aes-dupla-2026-10-19.json

## 20 — Um único bit muda muitos bits do resultado

RESULTADOS DA DUPLA

Um único bit muda muitos bits do resultado

Avalanche: contar as diferenças entre os dois textos cifrados.

MUDANÇA NA CHAVE

142 / 256 bits · 55,47%

MUDANÇA NA MENSAGEM · SOMENTE BLOCO 1

69 / 128 bits · 53,91%

O bloco 2 ficou igual no segundo teste: em ECB, os blocos são independentes.

AES-128 · AUGUSTO + GUSTAVO

20 / 27

**Notas de fala:** Bit 0 corresponde a 0x80 do primeiro byte. Com bit 0 da chave alterado: 72/128 no bloco 1 e 70/128 no bloco 2, total 142/256=55,47%. Com bit 0 da mensagem: 69/128 no bloco 1, 0/128 no bloco 2; o total é 69/256=26,95%. Isso é esperado no ECB, pois um bloco não afeta o outro. ~50% é uma expectativa estatística, não exigência exata para um ensaio.

**Fontes locais:** Codigo/evidencias/aes-dupla-2026-10-19.json · avalanche

## 21 — Por que o ECB revela padrões?

LIMITES DO MODO ECB

Por que o ECB revela padrões?

Com a mesma chave, blocos iguais produzem blocos cifrados iguais.

BLOCO 1

LIGAR LED VERDE.

e0c22a6f … 42203dec

BLOCO 2

DESLIGAR LED

2b4d065e … 4cd5070

BLOCO 3

LIGAR LED VERDE.

e0c22a6f … 42203dec

Um observador consegue reconhecer repetições mesmo sem conhecer o texto.

AES-128 · AUGUSTO + GUSTAVO

21 / 27

**Notas de fala:** Este experimento usa a chave fictícia do PDF, 2023100120231002, não a chave real da dupla. São três blocos exatos sem PKCS#7. Não há controle físico de LED implementado: as frases são apenas dados. Ciphertext repetido e0c22a6f02664f487dc9528642203dec; outro 2b4d065ea65478b60696616244cd5070.

**Fontes locais:** AES_Trabalho.pdf, p. 2; Codigo/src/vetores_pdf.c; logs aes-dupla

## 22 — Cifrar não garante que ninguém alterou os dados

LIMITES E ESCOLHAS

Cifrar não garante que ninguém alterou os dados

ECB não autentica: um atacante pode repetir ou trocar blocos.

NO TRABALHO

ECB + PKCS#7

Ajuda a estudar a cifra e enxergar blocos
repetidos.

EM UM PRODUTO

Um modo autenticado, como GCM, com chave
secreta e nonce único por chave.

Padding inválido pode revelar um erro. Padding válido não comprova integridade.

AES-128 · AUGUSTO + GUSTAVO

22 / 27

**Notas de fala:** O teste com chave errada no ensaio de matrículas fictícias levou a padding inválido, mas isso não é garantia de detectar toda corrupção. Cabeçalho e dados não são autenticados. CBC ou CTR exigem IV/nonce adequado e autenticação separada; GCM também autentica. Não trocar ECB exigido no trabalho.

**Fontes locais:** AES_Trabalho.pdf, p. 2; Codigo/src/receptor.cpp; aes-ensaio-2026-10-19.json

## 23 — O que conseguimos demonstrar

CONVERSA COM A TURMA

O que conseguimos demonstrar

O código faz as transformações e as placas recuperam a mensagem da dupla.

01

A transforma o texto em bytes cifrados.

02

A UART transporta o quadro binário.

03

B recupera exatamente a mensagem original.

Pergunta para a turma: se dois blocos forem iguais, o que o ECB deixa aparecer?

AES-128 · AUGUSTO + GUSTAVO

23 / 27

**Notas de fala:** Convidar professor a escolher uma mensagem nova ou um bit da chave. Cada integrante deve explicar trecho escrito pelo colega; autoria de cada arquivo ainda deve ser definida pela dupla. O guia de execução completo e o ensaio formal na sala seguem pendentes.

**Fontes locais:** Codigo/evidencias/aes-dupla-2026-10-19.json; Apresentacao/roteiro.md

## 24 — Apoio: os cinco estados do primeiro bloco

APÊNDICE · ETAPA 3

Apoio: os cinco estados do primeiro bloco

Valores reais da rodada 1. Hexadecimal na ordem dos bytes de entrada/saída.

1 · AddRoundKey inicial

76020604020300040a030c040403060a

2 · SubBytes

38776ff2777b63f2677bfef2f27b6f67

3 · ShiftRows

387bfe67777b6ff2677b6ff2f27763f2

4 · MixColumns

64b00d03fec2df72ded2cf42f74b4ee6

5 · AddRoundKey · chave 1

711efe2fdb5c1d6dc97b3569d4d081fc

AES-128 · AUGUSTO + GUSTAVO

24 / 27

**Notas de fala:** Os cinco estados são exigidos no relatório. Nesta tabela a saída de cada etapa aparece como sequência de 16 bytes. A matriz 4×4 nos diagramas é preenchida por colunas. Comando ETAPA3 exibe os valores, chave, padding e ciphertext.

**Fontes locais:** Codigo/evidencias/aes-dupla-2026-10-19.json · etapa3.bloco1_rodada1

## 25 — Apoio: a decifragem desfaz o caminho

APÊNDICE · CÓDIGO REAL

Apoio: a decifragem desfaz o caminho

A placa B usa as operações inversas com as chaves em ordem contrária.

aes128.c · dentro de aes128_decifrar_bloco()

  aes_add_round_key(s, ctx->chaves_rodada[AES128_RODADAS]);
  for (int r = AES128_RODADAS - 1; r >= 0; --r) {
    aes_inv_shift_rows(s);
    aes_inv_sub_bytes(s);
    aes_add_round_key(s, ctx->chaves_rodada[r]);
    if (r != 0) aes_inv_mix_columns(s);
  }

AES-128 · AUGUSTO + GUSTAVO

25 / 27

**Notas de fala:** Explicar InvShiftRows, InvSubBytes e InvMixColumns. AddRoundKey é a própria inversa. A posição da chave em relação a InvMixColumns respeita o algoritmo implementado. Trecho literal do código entregue.

**Fontes locais:** Codigo/src/aes128.c · aes128_decifrar_bloco()

## 26 — Apoio: encontre a resposta que não chegou

APÊNDICE · DIAGNÓSTICO UART

Apoio: encontre a resposta que não chegou

Atividade complementar com os logs históricos do diagnóstico sem criptografia.

B · RESPONDEU

1.000 PINGs recebidos
1.000 PONGs registrados no envio

A · CONTOU

999 PONGs recebidos
Faltou a sequência 1

Pergunta: o enlace funcionou? E passou no teste de zero perdas?

O enlace foi demonstrado nos dois sentidos. O resumo estrito terminou FAIL.

AES-128 · AUGUSTO + GUSTAVO

26 / 27

**Notas de fala:** Execução 5f7a5b57. B registrou 1000 PINGs e envio de 1000 PONGs. A registrou 999 PONGs, faltando sequência 1. Houve RX_ERROR no boot; descarte pelo parser é uma hipótese. RTT médio 5554,28 µs se refere somente a este diagnóstico, não ao AES. Esta atividade é apoio, não substituir a demonstração AES exigida.

**Fontes locais:** Codigo/evidencias/uart-pingpong-2026-10-19.json; uart-pingpong-a/b-2026-10-19.log

## 27 — Fontes e preparação final

APÊNDICE · REFERÊNCIAS

Fontes e preparação final

O material usa os arquivos locais do trabalho e os logs já existentes.

FONTES

Enunciado: AES_Trabalho.pdf

Código: Codigo/src/

Evidências: aes-dupla e aes-ensaio, 19/10/2026.

ANTES DA AULA

Revisar o código do colega.

Concluir o README de execução.

Ensaiar os dois monitores e a alteração pedida
pelo professor.

AES-128 · AUGUSTO + GUSTAVO

27 / 27

**Notas de fala:** Referência do algoritmo: NIST FIPS PUB 197 (2001), conforme o PDF do professor. README de execução da demonstração: requisito aprovado, documento e ensaio ainda pendentes. Os slides e suas notas são editáveis. Não apresentar este material como um novo ensaio de laboratório.

**Fontes locais:** AES_Trabalho.pdf; Codigo/src; Codigo/evidencias; Apresentacao/roteiro.md
