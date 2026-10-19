# Código e validação

O enlace físico UART foi demonstrado antes de AES. A sequência capturada teve
uma resposta perdida em 1.000; repetir com 1.000/1.000 antes de considerar a
validação de confiabilidade concluída. AES ainda não foi implementado.
O estado real está no [registro de laboratório](../Relatorio/validacao-laboratorio.md).

## Teste UART preparado

O [platformio.ini](../platformio.ini) na raiz define `esp_a` e `esp_b`, com fontes
em [src/main.cpp](src/main.cpp). A identificação física foi corrigida pelo usuário
após desconectar uma placa: **A é USB location 3-3; B é 3-1**. Os nomes Linux
trocaram após nova reconexão: A ficou em `/dev/ttyUSB0` e B em `/dev/ttyUSB1`.
Revalidar portas a cada reconexão. Leituras
reais da flash identificaram A como ESP32-D0WD-V3 v3.0 e B como ESP32-D0WD-V3
v3.1; ambos pertencem à família configurada por `esp32dev`. O usuário leu a
marcação **ESP32-WROOM-32** nas duas blindagens. GPIO16/17 estão disponíveis
nesse módulo, conforme o [guia de conexão da Espressif](https://docs.espressif.com/projects/esp-at/en/latest/esp32/Get_Started/Hardware_connection.html).
Ver os [logs de A](evidencias/leitura-chip-a-pass.log) e
[B](evidencias/leitura-chip-b-pass.log).

O firmware usa UART2 115200/8N1, RX16/TX17, exclusivamente para PING/PONG;
USB/UART0 registra logs. A inicia 1.000 tentativas 15 s após o boot e pode repetir
com `RUN` seguido de LF na USB, quando ociosa. B responde com o mesmo identificador
e sequência. O resumo de A informa respostas, perdas por timeout de 250 ms,
erros e RTT mínimo/médio/máximo em microssegundos. Não há retransmissão.
O ensaio pode levar cerca de 256 s mais logs se não houver respostas.

O limite de linha é 79 caracteres; linhas inválidas/excedentes são descartadas
até LF. Fragmentos incompletos expiram em 250 ms. Esse protocolo textual serve
apenas para o diagnóstico UART; o protocolo AES posterior deve usar os bytes
brutos do enunciado. O boot inclui modelo/revisão/MAC, mas esses logs posteriores
não substituem a identificação anterior à primeira gravação.

Usar o PlatformIO MCP instalado para as operações, sempre com `projectDir` igual
à raiz absoluta do projeto e `environment` explícito. Não executar gravação
multiambiente. Fluxo: `list_devices`, `agent_resolve_target`, auditoria de pinos,
`agent_build_diagnose`/`build_project` com `background=true`, acompanhamento de
tarefa até estado terminal e aprovação MCP do alvo/artefato antes de upload.
Capturar as duas USB com bindings válidos e correlacionar PING/PONG e sequências.
As funções de monitor/escrita USB também devem passar pelo MCP.

O build A inicial com espressif32 6.9.0 gerou ELF, mas falhou no BIN por uma
incompatibilidade SCons. **No reteste, ambos os ambientes compilaram e geraram
ELF/BIN com sucesso usando 7.1.3**, com logs em `evidencias/build-esp_a-7.1.3-pass.log`
e `evidencias/build-esp_b-7.1.3-pass.log`. A rejeição de build B pelo host na
primeira tentativa foi superada no reteste.
A primeira captura de A, isolada, falhou por falta de permissão da porta.
Após o usuário liberar a ACL, o MCP abriu a porta e capturou o boot e mensagens
do firmware antigo de DFPlayer Mini. A consulta adicional para identificar o chip
foi rejeitada pelo host. Esse boot não é resultado do teste UART preparado.
Posteriormente, as leituras de identificação de A e B passaram; os dois
firmwares UART foram gravados e verificados pelo esptool. Os ensaios mais antigos
falharam. Com a ligação final TX2/GPIO17→RX2/GPIO16 cruzada e GND comum, B
recebeu 1.000/1.000 PINGs e respondeu todos; A recebeu 999/1.000 PONGs, sem
registrar a sequência 1. Houve um RX_ERROR no boot, possível causa ainda não
confirmada. O enlace foi demonstrado nos dois sentidos,
mas o firmware encerrou `FAIL` por essa perda. Loopback A recebeu 999/1.000 ecos.
Detalhes no [registro de laboratório](../Relatorio/validacao-laboratorio.md) e
nos arquivos `evidencias/uart-pingpong-*` e `evidencias/uart-loopback-a-pass-*`.
Nas capturas de 19/10 o monitor foi aberto por porta explícita (`serial_session_start`),
não por binding de `agent_resolve_target`; a identidade foi conferida pela USB
location e pelo MAC do boot, igual ao lido pelo esptool (bytes em ordem inversa).

Estrutura da implementação:

| Pasta | Conteúdo esperado |
| --- | --- |
| `src/` | Firmware atual de diagnóstico UART; AES ainda pendente |
| `testes/` | Vetores e futuros testes automatizados |
| `evidencias/` | Logs, resultados intermediários e validações nas placas |

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
| Integrante/matrícula A | PENDENTE |
| Integrante/matrícula B | PENDENTE |
| Modelo dos chips | A: ESP32-D0WD-V3 v3.0; B: ESP32-D0WD-V3 v3.1; módulos ESP32-WROOM-32 informados pelo usuário |
| Pinos TX/RX da UART2 | TX17/RX16; loopback 999/1.000; enlace bidirecional 1.000 PINGs recebidos em B e 999 PONGs em A |
| Ferramenta e versão | PlatformIO MCP; Core 6.2.0, Python 3.14.7 |
| Plataforma/core ESP32 e versão | espressif32 7.1.3; builds UART A/B PASS; chips identificados |
| Compilação | MCP build/diagnose, raiz do projeto, ambiente explícito, background |
| Gravação A/B | PASS: firmware de diagnóstico UART gravado e verificado pelo MCP/esptool |
| Comunicação física A↔B | PASS do enlace bidirecional; sequência 1.000/1.000 pendente (999 PONGs recebidos) |
| Comando de teste no computador | PENDENTE; validar vetores após implementação AES |

Os dados em [testes/vetores.json](testes/vetores.json) vêm do PDF. Os valores
de matrícula ali são fictícios e não substituem os dados da dupla.
