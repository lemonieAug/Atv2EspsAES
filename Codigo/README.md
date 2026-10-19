# Código e validação

O firmware do trabalho está implementado e foi validado nas duas placas reais
em 19/10/2026: AES-128 do zero em C, ECB, PKCS#7 e o quadro binário
`0xAA | n | 16·n` na UART2. O primeiro ensaio usou as matrículas fictícias do
enunciado; o segundo, com as matrículas reais de Augusto (A) e Gustavo (B), está
em [aes-dupla-2026-10-19.json](evidencias/aes-dupla-2026-10-19.json). O estado detalhado está no
[registro de laboratório](../Relatorio/validacao-laboratorio.md) e no
[resumo do ensaio](evidencias/aes-ensaio-2026-10-19.json).

## Organização do código

| Arquivo em `src/` | Conteúdo |
| --- | --- |
| `aes128.c/.h` | SubBytes, ShiftRows, MixColumns, AddRoundKey e inversas, `xtime`, S-box gerada a partir de GF(2⁸), expansão de chave (10 rodadas), cifra, decifra e os cinco estados da rodada 1 |
| `pkcs7_ecb.c/.h` | Preenchimento PKCS#7 com validação estrita e modo ECB |
| `dupla.c/.h` | Regras da seção 2 para chave e mensagem; **as matrículas ficam em `dupla.h`** |
| `quadro.c/.h` | Montagem do quadro e receptor por máquina de estados com tempo limite |
| `analise.c/.h` | Cifra de mensagem completa, inversão de bit e contagem de bits (avalanche) |
| `vetores_pdf.c/.h` | Valores esperados do PDF, usados no autoteste e nos testes do PC |
| `comum.cpp/.h` | UART2, autoteste, registros da Etapa 3, avalanche, experimento ECB e console USB |
| `transmissor.cpp` | Placa A (`esp_a`) |
| `receptor.cpp` | Placa B (`esp_b`) |
| `diag/uart_diag.cpp` | Diagnóstico PING/PONG em texto (`diag_a`/`diag_b`), sem criptografia |

Os arquivos `.c` não dependem do Arduino: o mesmo código roda nas placas e nos
testes do computador. Nenhuma biblioteca criptográfica é usada. O OpenSSL só foi
usado fora do código entregue, para conferir resultados.

**Ordem dos bytes:** o bloco de 16 bytes entra no estado em colunas, como no
FIPS-197 (`s[linha][coluna] = in[linha + 4·coluna]`). Todo registro em
hexadecimal segue a ordem de entrada/saída, que é também a ordem de transmissão.

## Comportamento nas placas

**Placa A (transmissor).** No boot executa o autoteste (FIPS-197 B e C.1, exemplo
fictício e os três blocos ECB do PDF, 11 itens). Depois imprime os registros da
Etapa 3, a avalanche com o bit 0 da chave e com o bit 0 da mensagem, e o
experimento ECB. Três segundos após o boot cifra a mensagem da dupla e envia o
quadro com `write()` de bytes brutos. Se o autoteste falhar, A recusa transmitir.

**Placa B (receptor).** Executa o mesmo autoteste e aguarda. Descarta bytes até
encontrar `0xAA`, lê `n` (rejeita `n = 0`) e exatamente `16·n` bytes. Abandona o
quadro se ficar mais de 100 ms sem bytes ou passar de 1 s no total (o maior
quadro, 4.082 bytes, leva cerca de 355 ms). Em seguida decifra em ECB, valida e
remove o PKCS#7 e mostra a mensagem como texto e em hexadecimal. Padding
inválido descarta a mensagem; padding válido não prova integridade, porque ECB
não autentica.

**Comandos no monitor serial (USB, 115200, fim de linha LF):**

| Placa | Comando | Efeito |
| --- | --- | --- |
| A | `ENVIAR` ou botão **BOOT** (GPIO0) | cifra e envia a mensagem atual |
| A | `MSG <texto>` | define uma mensagem nova (1 a 240 caracteres) e envia |
| A | `MSGRESET` / `MENSAGEM` | volta à mensagem da dupla / mostra a atual |
| A e B | `AUTOTESTE`, `ETAPA3`, `ECB` | repetem o autoteste, os registros da Etapa 3 e o experimento ECB |
| A e B | `CHAVE`, `BITCHAVE <0..127>`, `CHAVERESET` | mostram, invertem 1 bit ou restauram a chave desta placa |
| A e B | `AVALANCHE CHAVE <bit>`, `AVALANCHE MSG <bit>` | contam os bits do cifrado alterados por 1 bit invertido |
| B | `ESTADO` | contadores de quadros recebidos e com erro |

Bit `i` significa o bit de máscara `0x80 >> (i % 8)` do byte `i / 8`. Para a
arguição, `BITCHAVE` em A seguido de `ENVIAR` mostra B rejeitando o padding,
porque B continua com a chave da dupla.

## Matrículas da dupla

`src/dupla.h` contém A = 2023178440013 (Augusto) e B = 202127844251 (Gustavo),
com `DUPLA_DADOS_FICTICIOS 0`. Chave: `7844001327844251` (últimos 8 dígitos de
cada um, em ASCII). Mensagem: `A:2023178440013;B:202127844251` (30 bytes, mais
2 bytes `0x02` de PKCS#7). Para mudar os dados, edite esse arquivo, rode os testes
do computador e grave **as duas placas**, porque A e B precisam da mesma chave.

## Testes no computador

```sh
sh Codigo/testes/executar_testes.sh
```

O script compila `testes/teste_host.c` com o código de `src/`, usando
`-Wall -Wextra -Wpedantic -Werror`. Ele confere a S-box, `xtime`, os vetores
FIPS-197 B e C.1, as chaves de rodada 1 e 10 e os estados da rodada 1 do
Apêndice B, o exemplo fictício completo (chave, PKCS#7, cifrado, quadro de
34 bytes e recuperação) e o experimento ECB. Também testa as regras de
derivação da chave, os casos de borda do PKCS#7 e o receptor: lixo antes do
`0xAA`, timeout, `n = 0` e byte atrasado. Resultado de 19/10: 66/66 PASS
([log](evidencias/testes-host-2026-10-19.log)). É execução no computador, não
nas placas.

## Compilação e gravação

Use o PlatformIO MCP com `projectDir` igual à raiz do projeto e um ambiente
explícito por vez; **nunca grave as duas placas numa única operação**. Confirme
as placas pela USB location (A = `3-3`, B = `3-1`), porque os nomes
`/dev/ttyUSB*` trocam ao reconectar. Depois de reconectar, refaça a ACL da porta
(`sudo setfacl -m u:augusto:rw /dev/ttyUSBx`).

Cada gravação e cada escrita na serial exigem aprovação no painel de operador.
O ID aprovado deve ser repassado na mesma chamada (`approvalId` no upload,
`approval_id` na escrita), com os demais argumentos idênticos aos do pedido;
uma chamada sem o ID abre um pedido novo.

| Ambiente | Placa | Uso |
| --- | --- | --- |
| `esp_a` | A | transmissor AES |
| `esp_b` | B | receptor AES |
| `diag_a` / `diag_b` | A / B | diagnóstico PING/PONG; limpa o RX no início de cada `RUN` (compilado, ainda não gravado) |

## Histórico do enlace UART

Os chips foram identificados por leitura real da flash: A é ESP32-D0WD-V3 v3.0
(MAC `a8:42:e3:8f:a9:d0`) e B é ESP32-D0WD-V3 v3.1 (`d8:bc:38:fd:a8:80`), em
módulos ESP32-WROOM-32 informados pelo usuário. Antes do AES, o diagnóstico
PING/PONG comprovou o enlace TX2/GPIO17→RX2/GPIO16 cruzado com GND comum: B
recebeu 1.000/1.000 PINGs e A recebeu 999/1.000 PONGs. Os ensaios mais antigos
falharam com a fiação anterior. Detalhes em `evidencias/uart-*` e no registro
de laboratório.

## Contratos do enunciado

- Chave: somente dígitos das matrículas; últimos oito de A e de B, com zeros
  à esquerda se necessário; concatenar A+B e converter os 16 caracteres em ASCII.
- Mensagem: `A:<matrícula completa A>;B:<matrícula completa B>`.
- Padding: PKCS#7, sempre de 1 a 16 bytes, inclusive quando a entrada já tem
  tamanho múltiplo de 16.
- Cifra: AES-128, 10 rodadas, modo ECB.
- UART2: 115200 bps, 8N1; ligações TX→RX, RX→TX e GND comum.
- Quadro binário: `0xAA` + `n` de um byte + exatamente `16 × n` bytes cifrados.
- Receptor: sincronizar no marcador, ler com tempo limite, decifrar, validar e
  remover padding, e exibir a mensagem original no monitor serial.

## Identificação e dados ainda pendentes

| Item | Valor |
| --- | --- |
| Integrante/matrícula A | Augusto — 2023178440013 (informado pelo usuário em 19/10) |
| Integrante/matrícula B | Gustavo — 202127844251 (informado pelo usuário em 19/10) |
| Modelo dos chips | A: ESP32-D0WD-V3 v3.0; B: ESP32-D0WD-V3 v3.1; módulos ESP32-WROOM-32 informados pelo usuário |
| Pinos da UART2 | TX GPIO17 / RX GPIO16 (TX2/RX2 na serigrafia) |
| Ferramenta e versão | PlatformIO MCP 3.1.0; Core 6.2.0; espressif32 7.1.3 (Arduino) |
| Gravação A/B | `aes128-ecb-v1` gravado e verificado pelo esptool em 19/10 |
| Comunicação AES A→B | PASS com as matrículas fictícias e com as reais (19/10) |
| Registros da Etapa 3 com dados reais | Capturados em 19/10: [aes-dupla-2026-10-19.json](evidencias/aes-dupla-2026-10-19.json) |

Os dados em [testes/vetores.json](testes/vetores.json) vêm do PDF. Os valores
de matrícula ali são fictícios e não substituem os dados da dupla.
