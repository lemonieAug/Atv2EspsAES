# AES-128 em dois ESP32 via UART

Ambiente de trabalho organizado a partir do [enunciado](AES_Trabalho.pdf).
O algoritmo e os documentos finais ainda precisam ser desenvolvidos.
O teste UART inicial está preparado em `Codigo/src/main.cpp`, com ambientes
`esp_a`/`esp_b` no `platformio.ini` da raiz. Consulte o
[estado real da validação](Relatorio/validacao-laboratorio.md): o enlace UART
bidirecional foi comprovado, com 1 PONG perdido em 1.000; AES ainda não foi
implementado nem executado nas placas.

| Local | Responsabilidade | Ponto de entrada |
| --- | --- | --- |
| `Codigo/` | AES, transmissor, receptor, testes e evidências | [Instruções](Codigo/README.md) |
| `Relatorio/` | Relatório de até quatro páginas | [Modelo](Relatorio/relatorio.md) |
| `Apresentacao/` | Apresentação e demonstração nas placas | [Roteiro](Apresentacao/roteiro.md) |

As orientações para agentes estão em [AGENTS.md](AGENTS.md).
O acompanhamento está em [TAREFAS.md](TAREFAS.md).

## Claude Code e PlatformIO MCP

As instruções de projeto para Claude Code estão em [CLAUDE.md](CLAUDE.md),
e [`.mcp.json`](.mcp.json) configura o servidor PlatformIO com escopo do projeto.
O lançador usa a cópia do MCP instalada no cache do Codex nesta máquina; se não
a encontrar, usa `npx` com `platformio-mcp@3.1.0`. Para verificar a conexão,
abra Claude Code na raiz, aprove o servidor do projeto e consulte `/mcp` ou rode
`claude mcp list` no terminal.

Claude Code está instalado como CLI nativo, versão 2.1.303. A verificação
`claude doctor` não encontrou problemas de instalação, e `claude mcp list`
reportou o servidor `platformio` conectado a partir da raiz do projeto. Esses
comandos confirmam CLI e MCP; a autenticação da conta não foi verificada por um
comando de status dedicado nesta auditoria.

## Informações a definir

- Nomes e matrículas completas dos integrantes A e B.
- Prazo de entrega, não informado no PDF.
- Endereço do repositório GitHub, não informado no PDF.

Na sessão de laboratório foi utilizado o PlatformIO MCP instalado (Core 6.2.0)
com espressif32 7.1.3. Os builds `esp_a` e `esp_b` passaram e os firmwares UART
foram gravados nas duas placas. A é ESP32-D0WD-V3 v3.0 e B v3.1; o usuário
confirmou módulos ESP32-WROOM-32. O loopback de A recebeu 999/1.000 ecos e o
enlace entre placas recebeu 999/1.000 PONGs em A, com os 1.000 PINGs observados
em B. A troca física foi comprovada, mas o firmware marcou a sequência como
FAIL por uma perda no início. Veja as evidências e próximos passos em
[validação do laboratório](Relatorio/validacao-laboratorio.md). Mapeamento físico:
A em USB location 3-3 e B em 3-1; os nomes `/dev/ttyUSB*` podem trocar ao
reconectar.

## Ordem de execução

1. Corrigir o descarte da primeira linha RX após boot e repetir o PING/PONG até
   obter 1.000/1.000, preservando logs dos dois lados.
2. Implementar AES e validar os vetores FIPS-197 no computador.
3. Implementar PKCS#7, ECB e os dados derivados das matrículas quando disponíveis.
4. Validar a comunicação criptografada nas placas reais.
5. Registrar estados intermediários, avalanche e experimento ECB.
6. Finalizar relatório e apresentação usando as evidências obtidas.

O relatório e o roteiro podem ser preparados em paralelo com o código; os
resultados só devem ser preenchidos depois da validação correspondente.

## Planejamento aprovado da demonstração em sala

A demonstração será apresentada em dois monitores USB/serial lado a lado:
A exibirá a mensagem original, o preenchimento PKCS#7 e o ciphertext enviado;
B exibirá o ciphertext recebido e a mensagem recuperada. AES-128 do zero em
C/Arduino, ECB e PKCS#7 serão executados nas placas. A UART2 transportará o
quadro binário do PDF; o hexadecimal será apenas a representação nos logs USB.
Os detalhes e critérios estão no [roteiro](Apresentacao/roteiro.md).

Como requisito adicional aprovado pelo usuário, deverá ser entregue
`Apresentacao/README.md` com o passo a passo reproduzível para executar a
demonstração, incluindo preparação, gravação, abertura dos dois monitores,
início/repetição, conferência dos resultados e solução de problemas.
Esse README ainda está PENDENTE e deverá refletir o firmware final validado.

A aprovação abrange somente o planejamento: implementação, ensaio nas placas
e redação do guia executável continuam pendentes. Um visualizador Python
fica fora do escopo aprovado desta demonstração.
