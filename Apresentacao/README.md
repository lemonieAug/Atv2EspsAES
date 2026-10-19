# Como executar a demonstração na sala

Passo a passo para mostrar ao vivo as duas placas: A cifra a mensagem da dupla e
envia; B recebe, decifra e mostra o texto. Os exemplos de saída abaixo são
trechos reais dos ensaios de 19/10/2026
([A](../Codigo/evidencias/aes-dupla-a-2026-10-19.log),
[B](../Codigo/evidencias/aes-dupla-b-2026-10-19.log)).

> **Estado deste guia:** escrito em 19/10/2026 a partir dos ensaios realmente
> executados naquele dia, pelo PlatformIO MCP. Falta um ensaio completo feito
> **seguindo este guia do início ao fim** para considerá-lo validado; registre-o
> conforme a seção 9.

## 1. O que levar

| Item | Detalhe |
| --- | --- |
| Placa A — Augusto (transmissor) | ESP32-WROOM-32, chip ESP32-D0WD-V3 v3.0, MAC `a8:42:e3:8f:a9:d0` |
| Placa B — Gustavo (receptor) | ESP32-WROOM-32, chip ESP32-D0WD-V3 v3.1, MAC `d8:bc:38:fd:a8:80` |
| 2 cabos USB de **dados** | um por placa; cabos só de carga não aparecem como porta serial |
| 3 fios jumper | TX, RX e GND |
| Computador | PlatformIO (Core 6.2.0, plataforma `espressif32@7.1.3`) e um monitor serial |

Os slides estão em [slides/](slides/COMO_EDITAR.md) e o roteiro da fala em
[roteiro.md](roteiro.md).

## 2. Firmware e matrículas

As duas placas precisam do firmware da mesma versão do repositório, porque a
chave sai das matrículas gravadas em
[`Codigo/src/dupla.h`](../Codigo/src/dupla.h):

```c
#define DUPLA_MATRICULA_A "2023178440013"   // Augusto
#define DUPLA_MATRICULA_B "202127844251"    // Gustavo
#define DUPLA_DADOS_FICTICIOS 0
```

Em 19/10/2026 as placas foram gravadas com esses valores. Se `dupla.h` mudar, ou
houver dúvida sobre o que está gravado, grave **as duas placas de novo** (seção 4).
Não use as matrículas do exemplo do PDF: com elas o firmware imprime
`AVISO matriculas=FICTICIAS_DO_ENUNCIADO`.

## 3. Ligações (com as placas desligadas)

| De | Para |
| --- | --- |
| A **GPIO17** (serigrafia `TX2` ou `17`) | B **GPIO16** (`RX2` ou `16`) |
| B **GPIO17** (`TX2` ou `17`) | A **GPIO16** (`RX2` ou `16`) |
| A **GND** | B **GND** |

**Não use `RX0`/`TX0`.** Esses pinos são a serial da USB; ligados entre as placas,
o enlace não funciona e a gravação pode falhar. Esse foi o motivo dos primeiros
ensaios sem resposta (registro em
[validacao-laboratorio.md](../Relatorio/validacao-laboratorio.md)).

## 4. Identificar as portas e gravar (só se necessário)

1. Conecte as duas placas por USB. No Linux elas aparecem como `/dev/ttyUSB0` e
   `/dev/ttyUSB1`, mas **a ordem pode trocar a cada reconexão**. Identifique por:
   - USB location, no computador do laboratório: A = `3-3`, B = `3-1`
     (`ls -l /sys/bus/usb-serial/devices/`); ou
   - linha de boot no monitor: A mostra `papel=transmissor efuse_mac=d0a98fe342a8`,
     B mostra `papel=receptor efuse_mac=80a8fd38bcd8`.
2. Sem permissão na porta (`Permission denied`), libere para o seu usuário:
   `sudo setfacl -m u:$USER:rw /dev/ttyUSBx`. Repita após cada reconexão.
3. Grave **uma placa por vez**, conferindo a porta: `esp_a` em A e `esp_b` em B.
   - Pelo PlatformIO MCP (como foi feito em 19/10): `upload_firmware` com
     `environment` e `port` explícitos. Aprove o pedido no painel de operador e
     repasse o mesmo `approvalId` na chamada; veja
     [Codigo/README.md](../Codigo/README.md#compilação-e-gravação).
   - Pela linha de comando do PlatformIO, equivalente que **não foi executado**
     nesta preparação: `pio run -e esp_a -t upload --upload-port /dev/ttyUSBx`
     (e `esp_b` para B).
4. Confira no log de gravação o MAC da placa e as linhas `Hash of data verified`.

## 5. Abrir os dois monitores

Use **115200 bps**, final de linha **LF** (CR+LF também funciona) e uma janela
por placa, lado a lado: **A à esquerda, B à direita**. Exemplos: Serial Monitor do
PlatformIO/VS Code, `pio device monitor -e esp_a` e `-e esp_b` (não executado
nesta preparação), ou o monitor do MCP. Feche qualquer outro programa que esteja
usando a porta; uma porta só aceita um monitor por vez.

## 6. Roteiro ao vivo

1. **Aperte EN em B.** Espere ver:
   ```text
   [B] AUTOTESTE resumo status=PASS aprovados=11/11
   [B] PRONTO aguardando 0xAA; timeout entre bytes=100 ms total=1000 ms
   ```
2. **Aperte EN em A.** A imprime o autoteste, os registros da Etapa 3, a
   avalanche e o experimento ECB, e **3 s depois envia sozinha**:
   ```text
   [A] MENSAGEM_ORIGINAL tamanho=30 texto="A:2023178440013;B:202127844251"
   [A] CIFRADO_ENVIADO bloco 1: 2e3e460ec295383cec6a018765ff0531
   [A] CIFRADO_ENVIADO bloco 2: 30b51beef909b9d823300de92a5e9fb3
   [A] QUADRO_TX aa022e3e460ec295383cec6a018765ff053130b51beef909b9d823300de92a5e9fb3
   [A] ENVIO 1 fim bytes=34 escritos=34 blocos=2 marcador=0xaa
   ```
3. **Mostre B** recebendo os mesmos bytes e recuperando o texto:
   ```text
   [B] QUADRO 1 recebido marcador=0xaa n=2 bytes=34 descartados_antes=100
   [B] CIFRADO_RECEBIDO bloco 1: 2e3e460ec295383cec6a018765ff0531
   [B] CIFRADO_RECEBIDO bloco 2: 30b51beef909b9d823300de92a5e9fb3
   [B] PKCS7 status=valido removidos=2 bytes de valor 0x02
   [B] MENSAGEM_RECUPERADA tamanho=30 texto="A:2023178440013;B:202127844251"
   [B] COMPARACAO igual_a_mensagem_da_dupla=SIM
   ```
4. Para **repetir o envio**, digite `ENVIAR` no monitor de A ou aperte o botão
   **BOOT** de A.

`descartados_antes` diferente de zero logo depois de ligar ou gravar A é normal:
é ruído no fio enquanto A reinicia, e B o ignora até achar o `0xAA`.

**Critério de sucesso:** B mostra `QUADRO ... recebido` com os mesmos blocos
cifrados que A enviou, `PKCS7 status=valido` e o texto idêntico ao original, com o
mesmo tamanho. Lembre na fala que padding válido não prova integridade: ECB não
autentica.

## 7. Alterações pedidas pelo professor

Digite os comandos no monitor indicado e pressione Enter. `AJUDA` lista todos.

| Pedido | Comandos | O que deve acontecer |
| --- | --- | --- |
| Cifrar uma mensagem nova | em A: `MSG <texto>` (1 a 240 caracteres) | A cifra e envia na hora; B mostra o novo texto. `igual_a_mensagem_da_dupla=NAO` é **esperado**, porque B compara com a mensagem padrão |
| Voltar à mensagem da dupla | em A: `MSGRESET`, depois `ENVIAR` | B volta a mostrar `...=SIM` |
| Inverter um bit da chave | em A: `BITCHAVE <0..127>`, depois `ENVIAR` | cifrado completamente diferente; B normalmente responde `PKCS7 status=INVALIDO ... mensagem descartada` (em raros casos o padding sai válido por acaso e B mostra bytes sem sentido) |
| Desfazer a troca de bit | em A: `CHAVERESET` | `CHAVE ... bits_diferentes_da_chave_da_dupla=0` |
| Medir o efeito avalanche | em A ou B: `AVALANCHE CHAVE <bit>` ou `AVALANCHE MSG <bit>` | contagem de bits alterados por bloco e no total |
| Repetir registros | `ETAPA3`, `ECB`, `AUTOTESTE`, `CHAVE` | reimprimem os registros do relatório |

O bit `i` é o bit `0x80 >> (i % 8)` do byte `i / 8`; o bit 0 é o mais significativo
do primeiro byte. Exemplo real com a chave fictícia (ensaio de 19/10):

```text
[A] BITCHAVE bit=0 invertido na chave desta placa
[A] CHAVE ascii="\xb2023100120231002" hex=b2303233313030313230323331303032 bits_diferentes_da_chave_da_dupla=1
[B] PKCS7 status=INVALIDO motivo=valor_de_preenchimento_invalido; mensagem descartada
```

O professor pode conferir qualquer cifrado com uma implementação de referência, por
exemplo: `printf '%s' 'A:2023178440013;B:202127844251' | openssl enc -aes-128-ecb
-K 37383434303031333237383434323531 | od -An -tx1` (OpenSSL aplica o mesmo PKCS#7).

## 8. Se algo der errado

| Sintoma | Causa provável e ação |
| --- | --- |
| B não mostra nada depois do envio | fios em `RX0`/`TX0`, TX ligado em TX ou sem GND comum: desligue e confira a seção 3 |
| `QUADRO ERRO timeout recebidos=x de y` | contato ruim ou fio solto durante o envio; reencaixe e envie de novo |
| `QUADRO ERRO n=0 invalido` | ruído na linha, comum ao gravar ou reiniciar A; envie de novo |
| `PKCS7 status=INVALIDO` sem ter usado `BITCHAVE` | chaves diferentes: rode `CHAVE` nas duas placas; se `hex=` divergir, use `CHAVERESET` ou grave as duas com o mesmo `dupla.h` |
| `TX RECUSADO autoteste_falhou` em A | firmware ou dados corrompidos: veja qual `AUTOTESTE FAIL` apareceu e grave de novo |
| `AVISO matriculas=FICTICIAS_DO_ENUNCIADO` | placa gravada com o exemplo do PDF: corrija `dupla.h` e grave as duas |
| `Permission denied` ou porta ocupada | refaça a permissão (seção 4) e feche outros monitores |
| Comando sem efeito | final de linha desativado no monitor; ative LF |

Para isolar problemas só do fio, há o diagnóstico PING/PONG sem criptografia
(`diag_a`/`diag_b` no [platformio.ini](../platformio.ini)); depois dele, grave
`esp_a`/`esp_b` de novo.

## 9. Guardar as evidências do ensaio

1. Salve o texto completo dos dois monitores, do boot até o último envio.
2. Copie para `Codigo/evidencias/` com data e placa no nome, por exemplo
   `demo-a-AAAA-MM-DD.log` e `demo-b-AAAA-MM-DD.log`, acrescentando no topo
   porta, USB location e comandos digitados.
3. Registre em [Codigo/evidencias/README.md](../Codigo/evidencias/README.md) o que
   foi testado e o resultado, sem completar linhas que não foram capturadas.
4. Para aceitar este guia, marque em [TAREFAS.md](../TAREFAS.md) o item do README
   da demonstração indicando o ensaio que seguiu estes passos.
