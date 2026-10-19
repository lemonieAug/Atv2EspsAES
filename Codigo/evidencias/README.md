# Evidências de execução

Há evidências reais de identificação dos chips, builds, gravações e UART nas
placas. O enlace físico bidirecional foi demonstrado e o firmware AES foi
validado nas duas placas, primeiro com as matrículas fictícias do enunciado e
depois com as matrículas reais da dupla. Consulte o
[registro de laboratório](../../Relatorio/validacao-laboratorio.md).

- `preflight-2026-10-18.json`: respostas reais do MCP e correção dos papéis A/B.
- `build-esp_a-6.9.0-falha.log`: build candidato que gerou ELF e falhou no BIN.
- `monitor-usb1-permissao.log`: erro real de permissão, não saída do ESP32.
- `acesso-liberado-2026-10-18.json`: diagnóstico e captura após liberar a ACL.
- `monitor-a-firmware-anterior.log`: boot real de A e logs do firmware DFPlayer
  preexistente; não é execução do firmware UART deste projeto.
- `sha256-2026-10-18.json`: hashes das fontes/configuração e evidências em
  18/10, preservados como registro histórico. Documentos editados depois
  divergem desses hashes. Os itens `Codigo/ferramentas/painel-operador/*` são
  da cópia local de laboratório e não estão versionados no repositório.
- `build-esp_a-7.1.3-pass.log` e `build-esp_b-7.1.3-pass.log`: builds UART
  bem-sucedidos no reteste; comprovam compilação, não execução física.
- `reteste-2026-10-18-2352.json`: resultados MCP do reteste e diagnóstico das portas.
- `builds-uart-7.1.3-sha256.json`: hashes e tamanhos dos artefatos gerados.
- `aprovacao-leitura-a-2353.json`: alvo isolado de A e pedido real de aprovação
  MCP, ainda sem leitura nem gravação executadas. O acesso ao painel foi
  diagnosticado e o fluxo de operador corrigido posteriormente.
- `painel-operador-testes.json`: 14 verificações com dados isolados, sem alterar
  pedidos reais.
- `painel-operador-navegador.json` e `painel-administrador-augusto.png`: entrada
  real em painel de teste com papel de operador e identificação visível.
- `painel-operador-projeto-real.json`: consulta somente leitura ao armazenamento
  real mostrou o pedido de leitura de A ainda pendente.
- `painel-operador-aprovacao-isolada.json`: aprovação bem-sucedida de pedido
  fictício, com armazenamento separado do projeto real.
- `leitura-chip-a-2026-10-19.json` e `leitura-chip-a-falha-sync.log`: tentativa
  aprovada de leitura real da flash de A, interrompida por falta de resposta no
  caminho TX do CH340 para o ESP; nenhum chip foi identificado nesse ensaio.
- `leitura-chip-a-pass.json` e `leitura-chip-a-pass.log`: segunda leitura real
  identificou ESP32-D0WD-V3 v3.0 e conferiu a tabela de partições de A.
- `leitura-chip-b-2026-10-19.json` e `leitura-chip-b-pass.log`: leitura real
  identificou ESP32-D0WD-V3 v3.1; a tabela de firmware anterior difere da do
  projeto, sem impedir a identificação do chip.
- `upload-uart-esp_a-2026-10-19.json` e `upload-esp_a-pass.log`: gravação real
  do teste UART em A, com hashes de segmentos verificados pelo esptool.
- `upload-uart-esp_b-2026-10-19.json` e `upload-esp_b-pass.log`: gravação real
  do teste UART em B, com hashes de segmentos verificados pelo esptool.
- `uart-primeira-sequencia-2026-10-19.json` e os dois logs `uart-primeira-*`:
  primeira sequência física, FAIL (0/1.000 PONGs), com capturas parciais dos
  monitores e resumo completo emitido por A.
- `uart-reencaixe-parcial-2026-10-19.json` e `uart-reencaixe-a-captura.json`:
  segunda tentativa parcial após reencaixe; sem PONG capturado em A e sem bytes
  recebidos pelo monitor de B durante a janela observada.
- `uart-loopback-a-fail-2026-10-19.log` e `.json`: execução física após reboot de
  A, firmware `uart-only-v1`, RX16/TX17 a 115200 8N1; 1.000 PINGs sem recepção,
  1.000 timeouts, 149 erros de linha, sem linha inesperada. Resultado FAIL.
- `sha256-retomada-2026-10-19.json`: hashes SHA-256 da configuração, código,
  documentação e evidência do loopback desta retomada.
- `uart-loopback-a-fail-2-2026-10-19.log` e `.json`: segundo loopback físico de
  A (~01:17-01:21 UTC), tentativa informada como jumper GPIO17→GPIO16, sem fios
  interplacas; esclarecimento posterior tornou incerta a associação física dos
  jumpers. Sequência `c191a142`: 1.000 PINGs, 0 ecos, 1.000 timeouts, 130 erros,
  0 inesperados. Captura parcial (seq 246-1000) com resumo completo. Resultado FAIL.
- `uart-loopback-a-pass-2026-10-19.log` e `.json`: terceiro loopback de A
  (~01:25-01:30 UTC), após o usuário esclarecer que jumpers anteriores talvez
  estivessem em RX0/TX0 e mover para TX2/GPIO17→RX2/GPIO16. Captura desde o boot,
  sequência `16c6d40f`: 1.000 PINGs,
  999 ecos idênticos em ordem (faltou o eco 1; houve `RX_ERROR` antes do RUN,
  mas a causa do descarte não foi instrumentada),
  0 erros durante o ensaio. O firmware imprime `status=FAIL` porque espera
  `PONG B`; o critério do loopback é o eco. Resultado PASS para TX17→RX16 de A.
- `uart-pingpong-2026-10-19.json`, `uart-pingpong-a-2026-10-19.log` e
  `uart-pingpong-b-2026-10-19.log`: primeiro PING/PONG físico entre as placas
  (~20:40-20:42 UTC), com cruzamento TX2/RX2 e GND comum, capturado desde o boot nas
  duas USB. Sequência `5f7a5b57`: B recebeu e respondeu os 1.000 PINGs; A contou
  999 PONGs (A não registrou a sequência 1; houve `RX_ERROR` no boot, causa
  ainda sob investigação), 0 erros durante RUN, RTT
  5.503/5.554,28/6.583 µs (mín./média/máx.). O firmware imprime `status=FAIL` por
  essa única perda.
- `testes-host-2026-10-19.log`: testes **no computador** do código C de
  `Codigo/src` (66/66 PASS) e conferência externa do exemplo com OpenSSL.
  Não é execução nas placas.
- `build-aes-esp_a-2026-10-19.log`, `build-aes-esp_b-2026-10-19.log`,
  `upload-aes-esp_a-2026-10-19.log` e `upload-aes-esp_b-2026-10-19.log`: build e
  gravação reais do firmware `aes128-ecb-v1` pelo MCP, com hashes verificados
  pelo esptool e MAC de cada placa.
- `aes-ensaio-2026-10-19.json` e os logs `aes-ensaio-*`: primeiro ensaio AES nas
  duas placas (~21:23-21:35 UTC), **com as matrículas fictícias do enunciado**.
  Autoteste 11/11 em A e B; registros da Etapa 3, avalanche e experimento ECB em
  A; mensagem da dupla, mensagem nova de 36 bytes e reenvio pelo botão BOOT
  recuperados em B, com cifrados idênticos aos enviados; com 1 bit da chave
  invertido em A, B rejeitou o PKCS#7.
- `aes-dupla-2026-10-19.json`, `aes-dupla-a-2026-10-19.log`, `aes-dupla-b-2026-10-19.log`,
  `build-aes-dupla-*`, `upload-aes-dupla-*` e `testes-host-dupla-2026-10-19.log`:
  ensaio **com as matrículas reais** (A Augusto 2023178440013, B Gustavo
  202127844251), ~21:40-21:45 UTC. Testes do PC 71/71; autoteste 11/11 em A e B;
  registros da Etapa 3 (chave, PKCS#7, cifrado, chaves de rodada 0 e 1, cinco
  estados) e avalanche; B recuperou a mensagem da dupla nos dois envios.

Ao adicionar resultados, informe data, versão do código, ferramenta e comando
utilizados, dados de entrada e se a execução ocorreu no computador ou nas placas.
Para hardware, registre modelo das placas, pinos e configuração serial.

Resultados necessários:

- Vetores FIPS-197 B e C.1: valores esperados/obtidos e decifragem.
- Dados reais da dupla: chave, mensagem com PKCS#7, cifrado e chaves 0 e 1 em hex.
- Cinco estados do bloco 1: AddRoundKey inicial, SubBytes, ShiftRows,
  MixColumns e AddRoundKey com a chave 1.
- Avalanche: dado original, alteração de um bit, cifrados e contagem de bits.
- Experimento ECB com os três blocos do enunciado.
- Transmissão e recepção: quadro enviado e mensagem recuperada nas placas reais.

Os agentes de relatório e apresentação devem usar estes resultados e citar
os respectivos arquivos. Saídas esperadas copiadas do PDF não são evidência de execução.
