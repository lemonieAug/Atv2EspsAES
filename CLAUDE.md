@AGENTS.md

## Claude Code neste projeto

- Leia também `README.md` e `TAREFAS.md` para entender o estado atual antes de
  iniciar uma tarefa.
- Antes de alterar uma pasta de trabalho, leia o `AGENTS.md` local. As
  instruções específicas para `Codigo/`, `Relatorio/` e `Apresentacao/` têm
  orientações adicionais.
- O PlatformIO está configurado em `platformio.ini` na raiz. `esp_a`
  (transmissor) e `esp_b` (receptor) são o firmware AES; `diag_a`/`diag_b` são o
  diagnóstico PING/PONG. O código-fonte fica em `Codigo/src/`; os testes do
  computador rodam com `sh Codigo/testes/executar_testes.sh`.
- Consulte `Codigo/README.md` e `Relatorio/validacao-laboratorio.md` antes de
  qualquer operação com placas. A configuração contém portas observadas em uma
  sessão anterior; portas USB podem mudar após reconexões.
- O usuário autorizou neste trabalho identificar, compilar, gravar e testar as
  placas pelo PlatformIO MCP. Continue esse fluxo nos limites já autorizados;
  nunca use operação multiambiente para gravar as duas placas. Respeite as
  aprovações individuais exigidas pelo MCP e não as contorne.
- Não trate resultados antigos ou os vetores do PDF como validação de uma
  execução atual. Registre a origem e a data de cada evidência.
- Preserve o diagnóstico UART em `diag_a`/`diag_b`; ele não faz parte do
  firmware entregue, mas serve para isolar problemas do enlace.
- Use português na documentação e explique mudanças e verificações ao concluir.

## Retomada do laboratório — 19/10/2026

Leia `Relatorio/validacao-laboratorio.md` antes de retomar. Estado confirmado:

- A é USB location `3-3`; B é `3-1`. Na enumeração mais recente, A era
  `/dev/ttyUSB0` e B `/dev/ttyUSB1`; identifique por location após reconectar.
- Ambas são módulos ESP32-WROOM-32 com chip ESP32-D0WD-V3; A revisão 3.0, B
  revisão 3.1. Os builds `esp_a`/`esp_b` passaram em espressif32 7.1.3 e os
  firmwares UART foram gravados nas placas com verificação do esptool.
- **Atualização ~21:35 UTC:** as placas estão com o firmware AES
  `aes128-ecb-v1` (A transmissor, B receptor), validado fisicamente com as
  matrículas FICTÍCIAS do enunciado: autoteste 11/11 nas duas placas, mensagem
  recuperada em B, mensagem nova via `MSG`, chave com 1 bit invertido rejeitada e
  reenvio pelo botão BOOT. Ver `Codigo/evidencias/aes-ensaio-2026-10-19.json`.
  Gravação e escrita serial exigem aprovação no painel; passe o ID aprovado na
  mesma chamada (`approvalId` no upload, `approval_id` na escrita).
- Histórico: antes do AES, o firmware era só o teste UART de texto. Os ensaios antigos
  falharam. Na captura mais recente, com fios cruzados em TX2/GPIO17→RX2/GPIO16
  e GND comum, B recebeu 1.000/1.000 PINGs e A recebeu 999/1.000 PONGs
  (`5f7a5b57`). Isso comprova o enlace físico nos dois sentidos, mas a sequência
  estrita ainda falha por uma perda. RTT válido em A: 5.503/5.554,28/6.583 µs.
  Consulte `Codigo/evidencias/uart-pingpong-2026-10-19.json` e os logs A/B.
- Loopback A em TX2/GPIO17→RX2/GPIO16 ecoou 999/1.000 PINGs (`16c6d40f`),
  comprovando o caminho local TX→RX. O resumo do firmware diz FAIL pois espera
  PONG B; para esse teste, o eco é o critério. As falhas anteriores podem ter
  usado RX0/TX0 segundo esclarecimento posterior do usuário; não afirme causa
  física mais específica que essa evidência.
- Próximo passo: com as matrículas reais em `Codigo/src/dupla.h`
  (`DUPLA_DADOS_FICTICIOS 0`), rodar os testes do PC, gravar `esp_a` e `esp_b`
  e capturar a Etapa 3 da dupla. Matrículas continuam PENDENTES; não as invente.
  Opcional: gravar `diag_a`/`diag_b` para a sequência estrita de 1.000/1.000.
- Todas as operações de build, flash e serial nas placas devem passar pelo
  PlatformIO MCP conforme `platformio-mcp:pio-manager`; não use `pio`, esptool ou
  monitor serial do shell para contornar o MCP.

O item de demonstração bidirecional pode permanecer concluído com a evidência
correlacionada registrada; mantenha pendente a sequência estrita de 1.000/1.000
até ela passar. Atualize evidências somente com dados capturados e distinga
execução em computador de execução física.

## Ferramenta

Claude Code CLI nativo 2.1.303 está instalado. Nesta auditoria, `claude doctor`
reportou que não há problemas de instalação e `claude mcp list` mostrou
`platformio` conectado na raiz do projeto. O doctor não confirma o estado de
autenticação da conta; não declare essa verificação concluída sem consultar a
sessão interativa do Claude Code.
