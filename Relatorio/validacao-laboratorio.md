# Validação de laboratório — UART e AES nas placas

Data: 18/10/2026 (America/Cuiaba). Evidências do MCP usam UTC.

## Firmware AES com as matrículas da dupla — 19/10/2026, ~21:40-21:45 UTC

Dupla informada pelo usuário: **A = Augusto, 2023178440013** (transmissor, `3-3`)
e **B = Gustavo, 202127844251** (receptor, `3-1`). `Codigo/src/dupla.h` foi
atualizado (`DUPLA_DADOS_FICTICIOS 0`), os testes do PC passaram 71/71 e as duas
placas foram gravadas individualmente, com aprovação e hashes verificados pelo
esptool. Execução física; resumo em
[aes-dupla-2026-10-19.json](../Codigo/evidencias/aes-dupla-2026-10-19.json).

| Registro (Etapa 3) | Valor obtido na placa A |
| --- | --- |
| Chave (ASCII / hex) | `7844001327844251` / `37383434303031333237383434323531` |
| Mensagem | `A:2023178440013;B:202127844251` (30 bytes) |
| Com PKCS#7, blocos 1 e 2 | `413a323032333137383434303031333b` · `423a3230323132373834343235310202` |
| Cifrado, blocos 1 e 2 | `2e3e460ec295383cec6a018765ff0531` · `30b51beef909b9d823300de92a5e9fb3` |
| Chave de rodada 0 | `37383434303031333237383434323531` |
| Chave de rodada 1 | `15aef32c259ec21f17a9fa2b239bcf1a` |
| (1) AddRoundKey inicial | `76020604020300040a030c040403060a` |
| (2) SubBytes | `38776ff2777b63f2677bfef2f27b6f67` |
| (3) ShiftRows | `387bfe67777b6ff2677b6ff2f27763f2` |
| (4) MixColumns | `64b00d03fec2df72ded2cf42f74b4ee6` |
| (5) AddRoundKey (chave 1) | `711efe2fdb5c1d6dc97b3569d4d081fc` |
| Avalanche, bit 0 da chave | 142/256 bits (55,47%): bloco 1 72/128, bloco 2 70/128 |
| Avalanche, bit 0 da mensagem | 69/256 (26,95%): bloco 1 69/128 (53,91%), bloco 2 0/128 |

Autoteste 11/11 em A e B. B recebeu o quadro `aa02…` nos dois envios (após a
gravação e após o EN), validou o PKCS#7 (2 × `0x02`) e mostrou a mensagem
original. O OpenSSL, como referência externa, reproduziu o cifrado e o cifrado
com o bit 0 da chave invertido. Uma implementação Python escrita só para
conferência reproduziu a chave de rodada 1 e os cinco estados. Logs:
[A](../Codigo/evidencias/aes-dupla-a-2026-10-19.log) e
[B](../Codigo/evidencias/aes-dupla-b-2026-10-19.log).

## Firmware AES nas duas placas — 19/10/2026, ~21:17-21:35 UTC

Execução física com o firmware `aes128-ecb-v1` (commit `434a01f`) e as
**matrículas fictícias do enunciado** (A=20231001, B=20231002). As matrículas reais
continuam PENDENTES, e os registros abaixo validam o procedimento, não os dados
da dupla. Resumo completo em
[aes-ensaio-2026-10-19.json](../Codigo/evidencias/aes-ensaio-2026-10-19.json).

- **Testes no computador PASS:** 66/66 verificações do mesmo código C, incluindo
  os estados da rodada 1 do FIPS-197 Apêndice B
  ([log](../Codigo/evidencias/testes-host-2026-10-19.log)). Execução no PC.
- **Build e gravação PASS:** `esp_a` e `esp_b` compilados pelo MCP e gravados
  individualmente com aprovação. O esptool verificou os hashes e o MAC de cada
  placa (A `a8:42:e3:8f:a9:d0` em `3-3`, B `d8:bc:38:fd:a8:80` em `3-1`).
- **Autoteste nas placas PASS:** 11/11 em A e 11/11 em B (FIPS-197 B e C.1,
  exemplo fictício e três blocos ECB).
- **Transmissão A→B PASS:** B recuperou a mensagem `A:20231001;B:20231002` em três
  quadros de 34 bytes (`AA 02 ...`) com cifrado idêntico ao enviado e ao do PDF.
  Recuperou também uma mensagem nova de 36 bytes (`MSG`, 3 blocos, PKCS#7 12×`0c`)
  e o reenvio pelo botão BOOT. O ruído na TX2 de A durante gravação e boot foi
  descartado pela sincronização no `0xAA` (até 2.202 bytes, inclusive um `n=0`
  rejeitado).
- **Chave errada:** com `BITCHAVE 0` em A (`0x32`→`0xb2`), B recebeu outro
  cifrado e rejeitou o padding (`valor_de_preenchimento_invalido`).
- **Etapa 3, com os dados fictícios:** chave de rodada 1 `373411f4060421c5343413f6050423c4`;
  estados do bloco 1 `730a0003…`, `8f67637b…`, `8f7b0163…`, `ea195336…` e
  `dd2d42c2…`. Avalanche: bit 0 da chave alterou 131/256 bits (51,17%); bit 0 da
  mensagem alterou 61/128 no bloco 1 e 0 no bloco 2, efeito do ECB.
- **Experimento ECB da página 2 PASS** nas duas placas; bloco 3 = bloco 1.
- O OpenSSL, como referência externa, reproduziu os cifrados da mensagem de 36
  bytes com a chave correta e com a chave alterada.

Logs: [A](../Codigo/evidencias/aes-ensaio-a-2026-10-19.log),
[B](../Codigo/evidencias/aes-ensaio-b-2026-10-19.log),
[boot de B](../Codigo/evidencias/aes-ensaio-b-boot-2026-10-19.log) e
[botão de A](../Codigo/evidencias/aes-ensaio-a-botao-2026-10-19.log). Os monitores
mostram registros dos firmwares, não uma captura elétrica independente da UART2.
A repetição com as matrículas reais está na seção anterior. O diagnóstico
PING/PONG corrigido (`diag_a`/`diag_b`) foi compilado, mas não gravado.

## Estado consolidado após retomada — 19/10/2026

Esta seção atualiza os estados anteriores deste documento; o histórico abaixo
preserva as tentativas e falhas na ordem em que ocorreram.

- **Identificação PASS:** A = ESP32-D0WD-V3 v3.0, B = ESP32-D0WD-V3 v3.1; o
  usuário confirmou ESP32-WROOM-32 nas duas. `esp32dev` e GPIO16/17 são
  compatíveis com o módulo identificado.
- **Build PASS:** ambientes `esp_a` e `esp_b`, espressif32 7.1.3.
- **Gravação PASS:** os dois firmwares UART de diagnóstico foram gravados e
  verificados pelo esptool; consulte os logs de upload abaixo.
- **UART entre placas PASS do enlace, mas FAIL no critério estrito de 1.000/1.000:**
  na execução mais recente B recebeu os 1.000 PINGs e respondeu todos; A recebeu
  999 PONGs; a sequência 1 está ausente em A, e houve um RX_ERROR no boot. Logs
  correlacionados comprovam os dois sentidos. O firmware reportou FAIL por uma
  perda; repetir após tratar o RX inicial. Tentativas antigas falharam por completo.
- **Loopback isolado A FAIL:** após EN, o monitor capturou novo boot e
  `UART_READY role=A rx=16 tx=17 baud=115200 format=8N1`. A sequência real
  `run_id=dc24342d` terminou em 255,011 s: 1.000 PINGs enviados, 0 recebidos,
  1.000 perdas, 149 erros de linha, 0 inesperados e sem latência calculável.
  O loopback foi informado pelo usuário; a captura não identifica por si só a
  causa elétrica. Ver [log completo](../Codigo/evidencias/uart-loopback-a-fail-2026-10-19.log)
  e [resumo JSON](../Codigo/evidencias/uart-loopback-a-fail-2026-10-19.json).
- **Segundo loopback isolado A FAIL (~01:17-01:21 UTC):** depois de corrigir o
  `.mcp.json` (o `${CLAUDE_PROJECT_DIR}` não era expandido e o MCP não iniciava),
  o usuário confirmou o jumper GPIO17→GPIO16 em A sem fios interplacas; B ficou
  só no USB. A estava em `/dev/ttyUSB0`/`3-3`, com leitura/escrita liberadas. A
  sequência automática `c191a142` já estava em andamento quando o monitor foi
  anexado (captura a partir de `seq=246`). Resumo do firmware: 1.000 enviados,
  0 recebidos, 1.000 perdas, 130 erros de linha, 0 inesperados, 255,021 s. Nenhum
  eco do próprio PING chegou ao RX16 (`RX_UNEXPECTED` ausente). Em esclarecimento
  posterior, o usuário disse que os jumpers anteriores talvez estivessem nos pinos
  RX0/TX0; a fiação física exata deste ensaio é incerta. Os `RX_ERROR` não
  identificam a causa. Ver [log](../Codigo/evidencias/uart-loopback-a-fail-2-2026-10-19.log)
  e [resumo](../Codigo/evidencias/uart-loopback-a-fail-2-2026-10-19.json).
- **Terceiro loopback A PASS (~01:25-01:30 UTC):** depois de esclarecer que os
  jumpers usados em ensaios anteriores podem ter ficado nos pinos RX0/TX0 (GPIO3/
  GPIO1, UART0 do USB), o usuário moveu o jumper para TX2/GPIO17→RX2/GPIO16.
  Com captura desde o boot (EN), a sequência `16c6d40f` enviou 1.000 PINGs e
  recebeu 999 ecos idênticos e em ordem como `RX_UNEXPECTED`, com 0 erros de linha
  durante o ensaio. Faltou só o eco 1: o `RX_ERROR` do boot ativa o descarte até o
  próximo LF (hipótese consistente com `main.cpp`, não comprovada por instrumentação).
  O `SUMMARY` diz `status=FAIL`
  porque o firmware espera `PONG B`; para o loopback, o critério é o eco. Isso
  valida só TX17→RX16 de A; naquele momento o PING/PONG entre placas ainda estava
  pendente (executado depois, ver item seguinte). As
  ligações entre placas dos ensaios anteriores também devem ser conferidas quanto
  ao uso de RX0/TX0. Ver [log](../Codigo/evidencias/uart-loopback-a-pass-2026-10-19.log)
  e [resumo](../Codigo/evidencias/uart-loopback-a-pass-2026-10-19.json).
- **PING/PONG físico entre placas PASS no enlace (19/10, ~20:40-20:42 UTC):** com
  A TX2/17→B RX2/16, B TX2/17→A RX2/16 e GND comum, ambos os monitores capturaram
  desde o boot (A `/dev/ttyUSB0`/`3-3`, B `/dev/ttyUSB1`/`3-1`). Na sequência
  `5f7a5b57`, B registrou os 1.000 PINGs válidos, em ordem, e respondeu todos; A
  contou 999 PONGs, com 0 erros de linha e 0 inesperados durante o ensaio. O PONG 1
  foi enviado por B, mas não aparece como recebido em A. Um `RX_ERROR` ocorreu no
  boot; o descarte da primeira linha por estado parcial do parser é hipótese
  consistente com o código, sem instrumentação que prove causalidade. RTT em A:
  mínimo 5.503 µs, média 5.554,28 µs, máximo 6.583 µs, em
  10,247 s. O `SUMMARY` diz `status=FAIL` por essa única perda; os dois sentidos
  físicos estão comprovados por logs correlacionados. Ver
  [resumo](../Codigo/evidencias/uart-pingpong-2026-10-19.json),
  [log A](../Codigo/evidencias/uart-pingpong-a-2026-10-19.log) e
  [log B](../Codigo/evidencias/uart-pingpong-b-2026-10-19.log).
- **AES e seus testes PENDENTES:** nenhum algoritmo foi implementado nesta etapa;
  matrículas completas ainda não foram fornecidas.

Na enumeração observada ao retomar, o inventário MCP mostrou A em
`/dev/ttyUSB0`/location `3-3` e B em `/dev/ttyUSB1`/location `3-1`, apesar de o
setup de loopback informado dizer que B estaria desconectada. Para a captura de
A, a localização USB identifica a placa sem depender do número da porta. Essa
observação é histórica; as capturas mais recentes usaram as duas placas e logs
desde o boot. Não altere a fiação com as placas energizadas.

O Claude Code deve seguir a retomada descrita em [`CLAUDE.md`](../CLAUDE.md).

Na execução `dc24342d`, A imprimiu `TX role=A text="PING A dc24342d 1"` e depois
`TIMEOUT`, repetindo até o pacote 1.000. Não houve `RX_UNEXPECTED` nem `RX role=A`,
portanto o loopback não passou. Os 149 `RX_ERROR` foram contabilizados pelo
firmware como linhas inválidas/excedentes; não provam que os PINGs foram recebidos.
O log bruto também contém um `RX_ERROR` antes do marcador `RUN`; o contador do
resumo foi zerado no início do ensaio e não inclui essa linha de boot.
RTT mínimo/médio/máximo ficaram sem amostra válida. Esta execução não valida
comunicação entre as placas.

## Reteste iniciado às 23:51 UTC

O usuário liberou o ambiente e solicitou nova tentativa. As chamadas MCP de
compilação agora executam. **Ambos os builds UART com espressif32 7.1.3 passaram**,
com geração de ELF e BIN e resultado final `SUCCESS`:

| Ambiente | Build | Tempo | RAM estática | Flash do programa |
| --- | --- | --- | --- | --- |
| `esp_a` | PASS | 2,488 s | 21.748 / 327.680 bytes | 271.457 / 1.310.720 bytes |
| `esp_b` | PASS | 2,458 s | 21.676 / 327.680 bytes | 297.233 / 1.310.720 bytes |

Esses valores são do compilador, não medições em hardware. As duas placas
voltaram a aparecer nas localizações confirmadas: A em USB1/3-3 e B em USB0/3-1.
`port_diagnose` confirmou acesso de leitura/escrita em A e negado em B. Foi
solicitado ao usuário liberar B e isolá-la para identificar A antes de gravar.
A identificação, gravação e teste de enlace ainda estavam pendentes nesse ponto
histórico; resultados posteriores estão consolidados no início deste documento.
As falhas da primeira tentativa registradas abaixo são históricas.

Atualização às 23:53 UTC: o usuário liberou o acesso de B e a nova consulta
`port_diagnose` confirmou leitura/escrita disponíveis. Depois o usuário desconectou
B; o inventário passou a listar somente A em USB1/3-3, removendo a ambiguidade
para o próximo acesso. A chamada de leitura da flash para obter a identificação
não foi rejeitada pelo host desta vez: chegou à política interna do MCP,
`flash_requires_approval`. Pedido `approval-c9ab63b7-24ce-493f-8262-62922a099dc9`,
operação `esp_flash_read` (classificada pelo MCP como `upload_firmware`),
pendente de aprovação do operador. **A leitura ainda não executou.**
Foi fornecido o painel de aprovação ao usuário; nenhuma política ou aprovação
foi alterada pelo agente. Ver [resposta de aprovação](../Codigo/evidencias/aprovacao-leitura-a-2353.json).

O usuário informou `Access denied: this dashboard session is invalid or has expired`
e repetiu a falha com um link autenticado recém-gerado. O endpoint local `/healthz`
respondeu HTTP 200 com `status=alive`, e a página raiz respondeu HTTP 200; isso
comprova disponibilidade HTTP, não autenticação do navegador ou aprovação.
Foram observadas seis instâncias do runtime PIO Agent e um listener na porta
8080, PID 71777. A relação dessas instâncias com a falha de sessão não foi
confirmada. Naquele momento não houve encerramento de processos nem alteração
na autenticação, política, pedidos ou armazenamento de aprovação. A identificação
e o ensaio físico permaneceram bloqueados; os dois builds continuaram PASS.

Diagnóstico posterior, solicitado pelo usuário: foi possível reproduzir o erro
sem cookie e abrir o painel com login novo em um perfil temporário do Zen.
As APIs então responderam 200 e o painel exibiu `SERVER: ONLINE`. Também foi
observado um bloqueio temporário HTTP 429, que cessou no teste seguinte. Isso
confirma que o login funciona no servidor; a causa da sessão inválida no perfil
do usuário ainda depende de conferir seu contexto de navegação. Não houve
aprovação de hardware. Ver o [diagnóstico detalhado](diagnostico-painel.md).

Às 00:08:53 UTC a leitura continuava `pending`. A inspeção do plugin 3.1.0
mostrou que os links MCP concedem visualização, enquanto a aprovação exige
autoridade de operador; o fluxo local instalado não fornece essa autoridade
pelas rotas de entrada examinadas. Portanto, abrir o painel não liberou a leitura.
Essa falha do fluxo de operador está documentada no diagnóstico detalhado.

Atualização em 19/10/2026: foi instalada uma correção local do fluxo de
operador, com backup para restauração. O painel de teste exibiu
`ADMINISTRADOR · augusto`; 14 testes de autorização passaram. Uma sessão de
operador no armazenamento real consultou os três pedidos pendentes, incluindo
a leitura de A, sem aprovar nenhum. O pedido ainda exigia aprovação manual às
00:17:18 UTC. A leitura, identificação exata do chip e PING/PONG físico
continuam **PENDENTE**. Ver [correção e testes](diagnostico-painel.md).

Às 00:24:41 UTC, após as aprovações específicas e a restauração da permissão
serial, o MCP executou a tentativa real de leitura de 4 KiB da tabela de partições
em A (`/dev/ttyUSB1`, localização 3-3). O `esptool.py` detectou o modo de
download, mas recebeu `getting no sync reply: The serial TX path seems to be
down`. A operação terminou `FLASH_READ_FAILED`, sem retornar dados da flash nem
identificar o modelo exato do chip. A causa física ainda depende de inspeção da
ligação RX0/GPIO3 e novo ensaio. Nenhum firmware foi gravado. Ver
[log real](../Codigo/evidencias/leitura-chip-a-falha-sync.log) e
[registro estruturado](../Codigo/evidencias/leitura-chip-a-2026-10-19.json).

Após retirar as ligações de RX0/GPIO3 e TX0/GPIO1 de A com a placa desligada,
o usuário reconectou o USB e restaurou a permissão serial. Às 00:29:05 UTC, uma
segunda leitura aprovada **PASSOU**: o `esptool.py` identificou
`ESP32-D0WD-V3` revisão `v3.0`, cristal de 40 MHz, CPU de 240 MHz e leu 4 KiB
em `0x8000`. O MCP encontrou seis partições e nenhuma diferença entre a tabela
lida e o artefato `esp_a`. O modelo pertence à família suportada por `esp32dev`.
Essa observação confirma a identidade eletrônica de A, mas ainda não verifica
o módulo da placa nem a disponibilidade física de GPIO16/17. Nenhum firmware
foi gravado. Ver [log](../Codigo/evidencias/leitura-chip-a-pass.log) e
[registro](../Codigo/evidencias/leitura-chip-a-pass.json).

Às 00:30:10 UTC, a leitura aprovada de B (`/dev/ttyUSB0`, localização 3-1)
identificou `ESP32-D0WD-V3` revisão `v3.1`, cristal de 40 MHz e CPU de 240 MHz.
Os 4 KiB foram lidos, mas o MCP marcou `device_table_mismatch`: B contém
partições de um firmware anterior (`phy_init`, `otadata`, `nvs`, `at_customize`,
`ota_0`, `ota_1`), diferentes das seis do artefato `esp_b`. A **identificação
física passou**; a comparação de layout falhou porque a placa ainda não recebeu
o firmware deste projeto. Nenhum firmware foi gravado. Ver
[log de B](../Codigo/evidencias/leitura-chip-b-pass.log) e
[resultado estruturado](../Codigo/evidencias/leitura-chip-b-2026-10-19.json).

O usuário leu a marcação **ESP32-WROOM-32** nas duas blindagens. O
[guia de conexão da Espressif](https://docs.espressif.com/projects/esp-at/en/latest/esp32/Get_Started/Hardware_connection.html)
mostra GPIO16 como RX e GPIO17 como TX nesse módulo. Com os chips D0WD-V3
identificados e os módulos WROOM informados, `esp32dev` e UART2 nos pinos
16/17 são uma configuração compatível para o teste; o funcionamento elétrico
das ligações entre placas ainda precisa de PING/PONG real.

Às 00:34 UTC, o MCP gravou o firmware UART `esp_a` em A e o `esptool.py`
verificou o hash dos segmentos escritos. O upload levou 27,43 s e terminou
`SUCCESS`. A iniciou o firmware e enviou PINGs no UART2; nesse momento B ainda
executava o firmware anterior, portanto os PINGs capturados expiraram. Isso
comprova o boot do novo firmware em A, mas ainda não o enlace bidirecional.
Ver [log do upload](../Codigo/evidencias/upload-esp_a-pass.log) e
[dados estruturados](../Codigo/evidencias/upload-uart-esp_a-2026-10-19.json).

Às 00:36 UTC, B recebeu o firmware UART `esp_b`; a escrita dos segmentos foi
verificada pelo `esptool.py` e o MCP retornou `SUCCESS` em 19,0 s. Isso substituiu
o firmware e a tabela de partições anteriores de B, conforme a aprovação do
usuário. Ver [log do upload](../Codigo/evidencias/upload-esp_b-pass.log) e
[dados estruturados](../Codigo/evidencias/upload-uart-esp_b-2026-10-19.json).
O enlace UART ainda exige confirmação por logs correlacionados dos dois lados.

O primeiro ensaio completo com ambos os firmwares terminou **FAIL**. O resumo
real emitido por A (`run_id=8052bb0e`) informou 1.000 PINGs enviados, zero
PONGs válidos, 1.000 perdas por timeout e 60 erros de linha em 255.995 ms. O
monitor de B mostrou `BOOT role=B` e `UART_READY role=B`, mas nenhum PING válido;
registrou três erros de linha na janela capturada. Os monitores foram anexados
depois do início da sequência, portanto os logs salvos são parciais; os totais
de 1.000 pacotes vêm do resumo do firmware, não de uma contagem das linhas
salvas. A ligação TX17→RX16 e o GND estão em conferência. Ver
[resultado estruturado](../Codigo/evidencias/uart-primeira-sequencia-2026-10-19.json),
[log parcial de A](../Codigo/evidencias/uart-primeira-sequencia-a-parcial.log) e
[log parcial de B](../Codigo/evidencias/uart-primeira-sequencia-b-parcial.log).

Depois de reencaixar os fios, o usuário religou as placas. A localização USB
continuou 3-3 e B continuou 3-1, mas o Linux inverteu os nomes das portas:
**A passou a `/dev/ttyUSB0` e B a `/dev/ttyUSB1`**. O `platformio.ini` foi
atualizado para essa enumeração observada. As ACLs temporárias também se
perderam na reconexão; o usuário restaurou a liberação de acesso.

Na captura parcial após reencaixar os fios, A continuou sem PONG válido: foram
capturados 205 PINGs, 204 timeouts e 10 erros de linha até o PING 394 da
sequência `1eb5d484`. O monitor de B recebeu zero bytes durante 58,9 s. Os
monitores foram encerrados antes do resumo para preparar um loopback local em A;
essa segunda sequência fica **PENDENTE**, sem extrapolar perdas para 1.000
pacotes. Ver [resultado](../Codigo/evidencias/uart-reencaixe-parcial-2026-10-19.json)
e [linhas capturadas de A](../Codigo/evidencias/uart-reencaixe-a-captura.json).

Evidências deste reteste:

- [Build A PASS](../Codigo/evidencias/build-esp_a-7.1.3-pass.log).
- [Build B PASS](../Codigo/evidencias/build-esp_b-7.1.3-pass.log).
- [Resultados MCP](../Codigo/evidencias/reteste-2026-10-18-2352.json).
- [Hashes e tamanhos dos artefatos](../Codigo/evidencias/builds-uart-7.1.3-sha256.json).

**Critério de UART física: enlace bidirecional PASS; sequência sem perdas PENDENTE.**
Os logs mostram 1.000 PINGs recebidos em B e 999/1.000 PONGs recebidos em A;
`SUMMARY` encerrou FAIL por uma perda. Loopback A comprovou TX17→RX16 com 999/1.000
ecos. Os firmwares UART foram gravados nas duas placas. AES ainda não foi
implementado; o critério criptográfico geral segue PENDENTE.

## Requisitos preservados

O [enunciado original](../AES_Trabalho.pdf), páginas 1–2, exige AES-128
implementado do zero em C/Arduino, sem biblioteca criptográfica, 10 rodadas,
ECB, PKCS#7 e UART2 a 115200 bps, 8N1. A chave concatena, em ASCII, os últimos
oito dígitos das matrículas A e B, com zeros à esquerda quando necessário.
A mensagem é `A:<matrícula A>;B:<matrícula B>` com matrículas completas.
O quadro é binário: `AA | n (1 byte) | exatamente 16*n bytes cifrados`.
As matrículas não foram fornecidas; o usuário informou não conhecê-las agora.
Os exemplos fictícios do PDF não são dados reais da dupla.

## Identidade e acesso

O primeiro inventário confirmou as portas e localizações informadas inicialmente.
Após a desconexão física, desapareceu USB0/3-1 e permaneceu USB1/3-3. O usuário
confirmou que a placa restante é A e que os rótulos A/B iniciais estavam invertidos.
O mapeamento corrigido é:

| Papel físico confirmado pelo usuário | Porta observada | USB location | USB VID:PID |
| --- | --- | --- | --- |
| A | `/dev/ttyUSB1` | `3-3` | `1A86:7523` |
| B (desconectada) | `/dev/ttyUSB0` antes da desconexão | `3-1` | `1A86:7523` |

As duas interfaces CH340 receberam o mesmo fingerprint do MCP:
`2adf32a3bc7a4bae918386daaf45b934ed74983bd4e91cf779732dc56592ad46`.
O resolvedor inicialmente declarou `confidence=exact`, porém ambas as capturas
com bindings foram recusadas com `AMBIGUOUS_TARGET`. Portanto, esse fingerprint
isolado não comprova uma identidade física distinta. É necessário conferir
novamente localização e porta após cada reconexão.

O campo `detectedBoard=uno` veio do inventário USB e não identifica o chip ESP.
O modelo exato, revisão, MAC e eventual PSRAM de ambos os microcontroladores permanecem
PENDENTES. A configuração `esp32dev` é candidata, não uma identificação medida.
GPIO16/RX e GPIO17/TX foram informados pelo usuário; sua compatibilidade final
depende do módulo. A auditoria estática MCP retornou `[]`, o que não inspeciona
o circuito físico. Módulos com PSRAM podem reservar esses pinos
([Espressif](https://docs.espressif.com/projects/esp-idf/en/v4.2.1/esp32/hw-reference/esp32/get-started-wrover-kit.html)).

Com apenas USB1 conectado, a ambiguidade desapareceu, mas o monitor falhou:

```text
UserSideException: [Errno 13] could not open port /dev/ttyUSB1:
[Errno 13] Permission denied: '/dev/ttyUSB1'
```

`port_diagnose` confirmou `permission.readable=false` e `writable=false`, sem
detentor observado. Embora a captura tenha retornado `success=true`, os 171 bytes
eram mensagens do monitor, não saída do ESP32. O monitor terminou inativo e o
lock estava livre. Foi orientado ao usuário executar, no host Linux,
`sudo setfacl -m u:augusto:rw /dev/ttyUSB1`. A eficácia depende de nova verificação;
essa orientação não é evidência de acesso liberado.

**Atualização às 23:41 UTC:** o usuário executou o comando sem erro. O diagnóstico
MCP subsequente confirmou `readable=true` e `writable=true`. Uma captura de cinco
segundos passou a abrir a porta e registrou 765 bytes, incluindo cabeçalho do
monitor e saída real do firmware preexistente:

```text
ets Jul 29 2019 12:21:46
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
entry 0x400805f0
Connecting to DFPlayer Mini failed!
button pressed
button pressed
```

O boot sugere a família ESP32 clássica, mas não fornece variante, MAC ou uso de
PSRAM; não valida sozinho a compatibilidade do módulo com GPIO16/17. Não houve
gravação nem troca de firmware. A consulta adicional via `partition_table`
(`readDevice=true`, leitura de 4 KiB para obter também o log de identificação do
esptool) foi rejeitada **antes de executar** pelo host com
`MCP tool call requires approval, but approval policy is never`.
Nenhum byte de flash foi lido por essa tentativa. Monitor ao final: inativo.
O problema de acesso do Linux foi resolvido para a porta atual; a consulta de
identificação e o fluxo de build/upload ainda dependem das permissões do host.

## Implementação e compilação

[platformio.ini](../platformio.ini) define `esp_a` e `esp_b`, portas explícitas
e [Codigo/src/diag/uart_diag.cpp](../Codigo/src/diag/uart_diag.cpp) (antes `Codigo/src/main.cpp`) contém o diagnóstico PING/PONG.
A envia 1.000 mensagens `PING A <id-hex-da-execução> <sequência>`; B responde
`PONG B <mesmo-id> <mesma-sequência>`. O identificador e a sequência são conferidos
em A; respostas inesperadas, perdas e erros de recepção são contabilizados.
Mensagens efetivamente transmitidas/recebidas: ver a sequência `5f7a5b57` no
estado consolidado (B 1.000/1.000 PINGs; A 999/1.000 PONGs).

UART2 usa GPIO16/RX, GPIO17/TX, 115200, 8N1; logs ficam na USB/UART0.
O teste inicia uma única vez, 15 s após o boot de A; o comando USB `RUN` permite
repeti-lo quando ocioso. Cada resposta tem limite de 250 ms, sem retransmissões;
o intervalo entre tentativas é 5 ms. A sequência é finita, cerca de 256 s mais
o custo de logs no pior caso sem respostas. O receptor usa linha de até 79
caracteres, descarta linhas inválidas/excedentes até LF e abandona quadros
incompletos após 250 ms. O RTT usa o relógio de A, incluindo transmissão,
processamento em B e retorno. Não é uma medição de latência unidirecional.

O build real de A usou PlatformIO Core 6.2.0/Python 3.14.7, espressif32 6.9.0,
framework Arduino ESP32 2.0.17 e SCons 4.11.1. Compilação e vínculo geraram ELF,
mas a geração de `firmware.bin` falhou com:

```text
*** [.pio/build/esp_a/firmware.bin] TypeError :
unsupported operand type(s) for +: '_Null' and 'str'
esp_a          FAILED    00:00:22.374
```

O ELF apontou RAM estática de 21.748/327.680 bytes e flash de
271.585/1.310.720 bytes. Esses valores são do build candidato e não são consumo
medido em placa. O MCP chegou a informar sucesso provisório enquanto o log ainda
instalava dependências; o histórico terminal registrou `status=error`, `exitCode=1`.
O diagnóstico final prevalece. IDs: command `f2c67fb0-d177-4354-b4dd-9982870a0699`,
task `ca617833-96b0-43a5-8099-21a89555acf4`.

A falha corresponde ao problema documentado no
[PlatformIO #1769](https://github.com/platformio/platform-espressif32/issues/1769).
A configuração foi ajustada para espressif32 **7.1.3**, já instalada pelo MCP;
o builder dessa versão usa `ELF2BINFLAGS`/`ELF2BINCMD`, separação introduzida pela
correção referenciada nessa issue. **A configuração alterada ainda não foi
compilada.** Não houve alteração em pacotes globais fora das operações MCP.

A chamada direta `build_project` para B foi rejeitada pelo host:

```text
MCP tool call requires approval, but approval policy is never
```

Essa rejeição não é erro do código nem aprovação concedida. Não foi contornada
por execução de compilação/gravação em shell ou por mudança de política.
O MCP declara perfil `flash_requires_approval`; a política de permissões do host
é independente. Nenhum pedido de gravação foi emitido, pois faltam identidade
do chip, binários válidos e acesso serial.

## Resultados

| Verificação | Resultado | Evidência ou impedimento |
| --- | --- | --- |
| Inspecionar PDF e requisitos AES | PASS | PDF original e vetores existentes |
| Enumerar CH340 e localizações iniciais | PASS | Respostas MCP preservadas |
| Corrigir rótulos físicos A/B | PASS | Desconexão observada e confirmação do usuário |
| Distinguir ambas por fingerprint MCP | FAIL | Mesmo hash; `AMBIGUOUS_TARGET` |
| Identificar chips reais e validar `esp32dev`/GPIO16/17 | PASS | A D0WD-V3 v3.0, B D0WD-V3 v3.1; módulos WROOM-32 informados; logs esptool |
| Criar ambientes e teste UART sem AES | PASS (implementação) | INI e firmware de diagnóstico; enlace físico aferido abaixo |
| Build A com espressif32 6.9.0 | FAIL | Exceção SCons na geração do BIN |
| Build após ajuste para 7.1.3 | PASS | Builds A e B com BIN, logs MCP em evidências |
| Build B | PASS | Build terminal bem-sucedido em espressif32 7.1.3 |
| Monitor USB1 com apenas A conectada | FAIL | `Errno 13`, sem saída da placa |
| Abrir USB1 após usuário liberar ACL | PASS | Leitura/escrita disponíveis; captura de boot real do firmware antigo |
| Gravar A/B | PASS | Upload MCP em ambas; hashes esptool verificados |
| PING A → B / PONG B → A, tentativas antigas (pinos incertos) | FAIL | Nenhum PONG válido nos dois ensaios iniciais; o mapeamento exato dos jumpers foi esclarecido depois |
| PING A → B / PONG B → A (TX2/RX2, 19/10 ~20:40 UTC) | PASS do enlace; FAIL 1.000/1.000 | `5f7a5b57`: B 1.000/1.000 PINGs; A 999/1.000 PONGs, 0 erros durante RUN; firmware `status=FAIL` pela seq 1 |
| Loopback local A, tentativas anteriores com fiação incerta | FAIL | `dc24342d` e `c191a142`: nenhum eco; esclarecimento posterior indica que jumpers podem ter sido postos em RX0/TX0 |
| Loopback local TX2→RX2 em A | PASS | `16c6d40f`: 999/1.000 ecos, 0 erros durante o ensaio |
| Sequência UART de 1.000 pacotes | FAIL (perda de 1 no ensaio mais recente) | Ensaio antigo perdeu tudo; execução final recebeu 999 PONGs e não registrou a seq 1; captura de reencaixe foi parcial |
| RTT mínimo/médio/máximo | PASS (medido) | `5f7a5b57`: 5.503 / 5.554,28 / 6.583 µs, 999 amostras, relógio de A |
| Perdas e erros | PASS (medidos) | Primeira tentativa: 1.000 perdidos/60 erros; execução final: 1 PONG perdido; loopback final: 1 eco ausente |
| AES-128 do zero, ECB e PKCS#7 | PENDENTE | Etapa posterior à validação UART física |
| Plaintext/ciphertext e recuperação exata nas duas placas | PENDENTE | Sem implementação/execução AES nesta etapa |
| Binários, comprimentos variados e mensagens diferentes | PENDENTE | Etapa posterior |
| 1.000 pacotes AES por direção e latência | PENDENTE | Etapa posterior |
| Chave incorreta e corrupção | PENDENTE | Etapa posterior; ECB não autentica |

## Continuação necessária

1. Investigar o byte/linha incompleto que gera `RX_ERROR` antes do RUN em A;
   revisar o descarte até LF no parser e corrigir/limpar o estado inicial de RX.
2. Rebuild/reflash individual via MCP e repetir PING/PONG com logs desde boot,
   visando 1.000/1.000 e resumo firmware PASS; conferir sequência 1 incluída.
3. Só depois implementar AES-128 do zero, validar FIPS B/C.1 e o exemplo
   fictício identificado, PKCS#7 e quadro ECB binário, preservando o enunciado.
4. Testar texto e todos os valores de byte, incluindo NUL/AA/FF, nos comprimentos
   0, 1, 15, 16, 17, 31, 32, 33, 255, 256 e nos limites de buffer escolhidos;
   repetir 1.000 pacotes por direção, verificando todos os bytes e sequências.
5. Medir RTT com relógio único e distinguir custo AES de transporte/logs.
   Testar chave errada, ciphertext alterado em blocos intermediários/finais,
   truncamento, padding inválido, repetição e troca de blocos.

Padding inválido pode causar rejeição, mas padding válido não comprova chave
correta nem integridade. ECB/PKCS#7 não fornece autenticação; erros detectados
por comparação com a entrada conhecida no ensaio não são detecção criptográfica
de adulteração. Nenhum desses ensaios está declarado como executado.

## Arquivos de evidência

- [Respostas reais do preflight](../Codigo/evidencias/preflight-2026-10-18.json).
- [Log completo do build A que falhou](../Codigo/evidencias/build-esp_a-6.9.0-falha.log).
- [Log real de erro do monitor](../Codigo/evidencias/monitor-usb1-permissao.log).
- [Atualização após liberar acesso](../Codigo/evidencias/acesso-liberado-2026-10-18.json).
- [Log de boot real de A, firmware antigo](../Codigo/evidencias/monitor-a-firmware-anterior.log).
- [Hashes SHA-256 dos arquivos e fontes](../Codigo/evidencias/sha256-2026-10-18.json).
- [Captura completa do loopback A que falhou](../Codigo/evidencias/uart-loopback-a-fail-2026-10-19.log)
  e [resumo](../Codigo/evidencias/uart-loopback-a-fail-2026-10-19.json).
- [Hashes desta retomada](../Codigo/evidencias/sha256-retomada-2026-10-19.json).

Este registro técnico complementa o [modelo acadêmico](relatorio.md), que permanece
com os dados e resultados acadêmicos pendentes. Não constitui demonstração em placa.
