# Acompanhamento do trabalho

Marcar como concluído somente quando houver implementação ou evidência verificável.

## Preparação

- [x] Criar as pastas Codigo, Relatorio e Apresentacao.
- [x] Documentar o escopo e as responsabilidades dos agentes.
- [x] Preparar modelos de relatório e roteiro da apresentação.
- [x] Transcrever os vetores do enunciado para um arquivo estruturado.
- [x] Preparar instruções do Claude Code e configuração de projeto para PlatformIO MCP.
- [x] Instalar Claude Code CLI nativo 2.1.303; `claude doctor` sem problemas e
      `claude mcp list` com PlatformIO conectado.
- [ ] Confirmar autenticação da conta Claude Code na sessão interativa.
- [x] Informar integrantes, ordem A/B e matrículas completas: A = Augusto, 2023178440013;
      B = Gustavo, 202127844251 (informado pelo usuário em 19/10).
- [x] Definir placas, pinos e ferramenta de desenvolvimento (ESP32-WROOM-32 informado; chip confirmado via esptool; UART2 RX16/TX17; PlatformIO MCP).
- [ ] Configurar compilação, dependências e instruções reproduzíveis.

## Código e validação

- [x] Criar ambientes PlatformIO `esp_a` e `esp_b` e implementar teste UART PING/PONG sem AES.
- [x] Identificar os chips reais e confirmar compatibilidade de `esp32dev` e GPIO16/17.
- [x] Liberar acesso serial para a sessão, compilar e gravar o teste UART nas duas placas (ACL temporária precisa ser refeita após reconexão USB).
- [x] Demonstrar primeiro PING/PONG físico com logs das duas USB (19/10 ~20:40 UTC,
      `5f7a5b57`: A→B 1.000/1.000, B→A 999/1.000; seq 1 ausente em A após RX_ERROR no boot;
      `Codigo/evidencias/uart-pingpong-2026-10-19.json`).
- [ ] Repetir a sequência UART após tratar o `RX_ERROR`/descarte no boot e obter
      1.000/1.000 PONGs, com resumo PASS e evidência nos dois monitores. A correção
      (limpar RX no `RUN`) está em `diag_a`/`diag_b`, compilada mas não gravada;
      o enlace já foi exercido pelo firmware AES (ver abaixo).
- [x] Diagnosticar o loopback local GPIO17→GPIO16 de A (após esclarecer a
      associação incerta dos jumpers anteriores, com TX2→RX2 PASS em 19/10
      ~01:25 UTC: 999/1.000 ecos,
      0 erros; ver `Codigo/evidencias/uart-loopback-a-pass-2026-10-19.json`).
- [x] Implementar SubBytes, ShiftRows, MixColumns e AddRoundKey (`Codigo/src/aes128.c`).
- [x] Implementar expansão de chave AES-128 e decifragem.
- [x] Validar os vetores FIPS-197 B e C.1 e a recuperação do texto claro: 66/66 testes
      no PC (`Codigo/evidencias/testes-host-2026-10-19.log`) e autoteste 11/11 em A e B
      (`Codigo/evidencias/aes-ensaio-2026-10-19.json`).
- [x] Implementar derivação da chave e mensagem com as matrículas (`Codigo/src/dupla.c`;
      valores em `Codigo/src/dupla.h`, hoje com as matrículas fictícias).
- [x] Implementar PKCS#7, sua validação e remoção, e ECB.
- [x] Conferir o exemplo fictício do enunciado (PC e nas duas placas).
- [x] Implementar o transmissor com quadro binário AA | n | 16*n bytes (`transmissor.cpp`).
- [x] Implementar receptor com sincronização, tempo limite e validação do quadro
      (`receptor.cpp`, `quadro.c`).
- [x] Validar transmissão e recuperação da mensagem nas duas placas reais: com as
      matrículas fictícias (19/10 ~21:23 UTC: mensagem nova, chave errada rejeitada,
      botão BOOT) e com as reais (19/10 ~21:43 UTC, `Codigo/evidencias/aes-dupla-2026-10-19.json`).
- [x] Capturar chave, mensagem preenchida, cifrado e chaves de rodada 0 e 1 com os
      dados da dupla (placa A, 19/10 ~21:43 UTC; `aes-dupla-2026-10-19.json`).
- [x] Capturar os cinco estados solicitados do primeiro bloco (idem; conferidos com
      uma implementação Python independente).
- [x] Executar avalanche e registrar contagem e percentual de bits alterados: bit 0 da
      chave 142/256 (55,47%); bit 0 da mensagem 69/128 no bloco 1 e 0 no bloco 2.
- [x] Executar os três blocos do experimento ECB da página 2 (chave do exemplo, nas duas
      placas; bloco 3 = bloco 1).
- [ ] Comentar e revisar o código do transmissor e receptor — código comentado; falta a
      revisão pela dupla.

## Entrega

- [x] Registrar diagnóstico real e impedimentos do laboratório em `Relatorio/validacao-laboratorio.md`.
- [x] Finalizar relatório com resultados e discussão de ECB (`Relatorio/relatorio.md`,
      7 diagramas, linguagem acessível).
- [x] Gerar PDF do relatório e verificar limite de quatro páginas (`Relatorio/relatorio.pdf`,
      4 páginas, conferidas pelo script `Relatorio/ferramentas/gerar_relatorio.py`).
- [ ] Preparar apresentação e explicação de cada transformação.
- [ ] Preparar e ensaiar a demonstração AES em dois monitores USB/serial:
      mensagem original, PKCS#7 e ciphertext enviado em A; ciphertext recebido
      e mensagem recuperada em B, conforme `Apresentacao/roteiro.md`.
- [ ] Entregar `Apresentacao/README.md` com requisitos, ligações, identificação
      das placas/portas, configuração das matrículas, compilação e gravação via
      MCP, abertura dos dois monitores, início/repetição, critérios de sucesso,
      captura de evidências e solução de problemas; validar o passo a passo
      em ensaio nas duas placas. Requisito adicional do usuário, com apenas
      o planejamento aprovado nesta etapa.
- [ ] Ensaiar explicação cruzada: cada integrante explica código do colega.
- [ ] Ensaiar mudança de mensagem e inversão de um bit durante a demonstração.
- [ ] Disponibilizar código comentado no GitHub quando solicitado.
- [ ] Fazer demonstração e arguição com as duas placas reais.
