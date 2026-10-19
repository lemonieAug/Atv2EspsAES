# Diagnóstico do acesso ao painel PlatformIO MCP

Data: 18/10/2026–19/10/2026 UTC; 18/10/2026 America/Cuiaba.

## Conclusão

Foi reproduzida a mensagem exata relatada pelo usuário ao acessar a página raiz
sem cookie de sessão. No mesmo navegador e no mesmo servidor, entrar por um
link autenticado recém-gerado criou a sessão e abriu o painel com `SERVER: ONLINE`.
Isso confirma o funcionamento do servidor e do fluxo de login em perfil limpo.
A causa imediata da mensagem é ausência de uma sessão válida no contexto de
navegação. Ainda não foi determinado por que o navegador do usuário não manteve
ou não recebeu essa sessão: não foi acessado seu perfil pessoal nem seus cookies.

## Teste controlado

Foi usado o Zen 1.23b/Gecko 157 instalado, em modo headless com perfil temporário,
sem extensões ou dados do perfil pessoal. A automação utilizou
[WebDriver BiDi](https://developer.mozilla.org/en-US/docs/Web/WebDriver/How_to/Create_BiDi_connection).
O navegador temporário foi encerrado e o perfil descartado após o teste.

| Ação | Resultado real |
| --- | --- |
| Abrir `http://127.0.0.1:8080/` sem sessão | Página HTML 200; APIs 401; mensagem exata `Access denied: this dashboard session is invalid or has expired.` |
| Entrar pela rota autenticada `/auth/launch` com novo ticket | HTTP 303 para `/`; sessão criada |
| Consultar comandos, workspaces e hardware após login | HTTP 200 |
| Conectar atualização do painel | WebSocket HTTP 101 e painel `SERVER: ONLINE` |
| Executar operação de hardware ou aprovar pedido | Não realizado |

Um teste HTTP separado também confirmou cookie `pio_mcp_session`, caminho `/`,
e autenticação aceita no canal Socket.IO. Nenhum cookie, ticket, token ou URL
autenticada foi persistido nas evidências.

## Regras observadas no código instalado

O runtime `platformio-mcp` 3.1.0 define:

- Ticket de entrada: **60 segundos**, uso único (removido ao consultar a rota).
- Cookie de sessão: **8 horas**, `HttpOnly`, `SameSite=Strict`, caminho `/`.
- A página raiz serve o frontend sem exigir login; o frontend mostra acesso
  negado quando as APIs ou a conexão recebem rejeição de autenticação.
- A URL raiz, por si só, não cria a sessão. Reabrir um link de entrada já usado
  ou expirado também não cria uma nova sessão.

Isso corrige a orientação inicial de abrir somente o endereço base: ele funciona
após autenticar, mas não substitui o link de entrada em um navegador sem sessão.

## Bloqueio adicional por volume de consultas

Durante o diagnóstico, APIs responderam HTTP 429 com a mensagem
`Too many requests from this IP, please try again after 5 minutes`.
O código limita `/api` a 100 requisições por janela de cinco minutos por origem
IP. Um cabeçalho `Retry-After` observado indicava 15 segundos restantes. Após
o vencimento dessa janela, o teste em navegador recebeu HTTP 200 nas APIs.

HTTP 429 é um bloqueio adicional, distinto dos HTTP 401 que reproduziram o erro
de sessão. Não se deve atribuir a mensagem de sessão inválida somente ao 429.
Fechar abas antigas do painel reduz consultas simultâneas enquanto se aguarda
o prazo informado pelo servidor.

## Instâncias do servidor

Foram observadas seis instâncias do runtime; somente PID 71777 escutava
`127.0.0.1:8080` durante a inspeção. Não foi demonstrado que as instâncias
adicionais causaram o erro: o login novo funcionou no servidor dessa porta.
Nenhuma delas foi encerrada e nenhuma configuração de autenticação foi alterada.

## Procedimento de acesso

1. Fechar abas antigas do painel. Se houver erro 429, aguardar o `Retry-After`.
2. Usar uma janela privativa de um navegador normal para isolar cookies antigos
   ou o contexto de navegação do aplicativo.
3. Abrir um link autenticado novo e completo antes de 60 segundos, uma única vez.
   Um link consumido pelo diagnóstico não deve ser fornecido ao usuário.
4. Após a entrada, manter a aba autenticada e o mesmo host (`127.0.0.1`).

O teste confirma acesso ao painel, não autoridade para aprovar hardware.
As aprovações do laboratório permanecem pendentes.

## Falha adicional identificada no fluxo de aprovação

Às 00:08:53 UTC, o MCP ainda retornava `status=pending` para
`approval-c9ab63b7-24ce-493f-8262-62922a099dc9` (`esp_flash_read`).
A inspeção estática do runtime instalado 3.1.0 identificou um bloqueio distinto
do login de visualização:

- `issueLaunchTicket(projectDir, operator = false)` cria, por padrão, um ticket
  sem autoridade de operador. `getDashboardStatusCore` chama
  `getDashboardStatus(input.open, input.projectDir)` sem passar essa autoridade.
- O marcador `OPERATOR_LAUNCH` aparece somente na declaração e na comparação
  dentro da geração do ticket; não foi encontrada chamada que o forneça para
  emitir uma sessão de operador.
- `requireApprovalAuthority` exige sessão de operador, origem válida e cabeçalho
  específico, ou um token de aprovação previamente configurado pelo operador.
  O processo que escutava a porta 8080 não tinha `PIO_MCP_APPROVAL_TOKEN`
  configurado. Somente a presença da variável foi verificada; nenhum segredo
  foi lido ou registrado.
- A negativa prevista pelo guard é HTTP 403, código
  `OPERATOR_APPROVAL_REQUIRED`, com orientação para `dashboard --operator` ou
  CLI local `approve/deny`. Entretanto, o entrypoint instalado não contém
  subcomandos `approve`/`deny`, e o caminho `dashboard` chama a geração padrão
  sem autoridade de operador.

Referência local: runtime `platformio-mcp.mjs`, linhas 127886, 127936–127944,
128008, 128034–128045, 129035–129037 e 132530–132534.
Esses fatos mostram por que o link comum permite visualizar o painel, mas não
completa a aprovação por esse fluxo. O retorno 403 é previsto pelo código;
não foi enviado POST de aprovação pelo agente para reproduzi-lo, e ainda não
foi capturada a resposta da tentativa do usuário no navegador.

Essa falha foi corrigida localmente em 19/10/2026, conforme a seção seguinte.
Não foram alteradas políticas, modificados registros de aprovação ou
automatizados cliques de aprovação.

## Correção local e validação

O comando local `dashboard --operator` agora emite um ticket de operador somente
quando executado em terminal interativo. Os links gerados pelas ferramentas MCP
mantêm o perfil de visualização. Uma rota de consulta autenticada informa o papel
da sessão à interface, que mostra `ADMINISTRADOR · augusto` quando aplicável.
As verificações do servidor que exigem origem e cabeçalho de aprovação foram
preservadas. Há backup e instruções de restauração em
`Codigo/ferramentas/painel-operador/README.md`, cópia local da máquina de
laboratório que não é versionada no repositório, por decisão do usuário.

Em armazenamento isolado, 14 verificações de autenticação e autorização
passaram. Uma sessão de operador alcançou a rota de aprovação para um ID
**inexistente** e recebeu HTTP 404; uma sessão comum recebeu HTTP 403. Sem o
cabeçalho exigido, o operador recebeu HTTP 403. Nenhum pedido real foi aprovado
ou alterado. Um navegador com perfil temporário confirmou o texto
`ADMINISTRADOR · augusto`, APIs HTTP 200 e `SERVER: ONLINE`.

No processo de operador iniciado com o armazenamento real, uma consulta GET
confirmou o papel `operator` e mostrou os três pedidos ainda pendentes no mesmo
repositório de aprovações do processo MCP. Entre eles estava a leitura da placa A
`approval-c9ab63b7-24ce-493f-8262-62922a099dc9`. Esse teste não aprovou
nem consumiu pedido algum.

O acesso de administrador ao painel está corrigido e testado localmente. A
identificação dos chips e a gravação dos firmwares continuam pendentes.

Após a correção, o usuário confirmou que o painel mostrou
`ADMINISTRADOR · augusto` e aprovou um pedido. O MCP registrou a aprovação real
de `approval-c76ecd6d-e8af-4323-bf0e-b11c904ed8fd`. A leitura de A recebeu
as duas aprovações exigidas e chegou ao `esptool.py`; o comando falhou na
sincronização serial. O [log desse ensaio](../Codigo/evidencias/leitura-chip-a-falha-sync.log)
mostra que a falha de acesso ao painel foi resolvida, enquanto a comunicação
física com o bootloader permanece em investigação. Nenhuma gravação ocorreu.

## Evidências

- [Resultados HTTP e texto das duas telas](../Codigo/evidencias/painel-autenticacao-2026-10-18.json).
- [Painel autenticado no perfil temporário](../Codigo/evidencias/painel-sessao-limpa.png).
- [Testes de autorização do operador](../Codigo/evidencias/painel-operador-testes.json).
- [Registro do navegador com operador](../Codigo/evidencias/painel-operador-navegador.json).
- [Painel mostrando o administrador](../Codigo/evidencias/painel-administrador-augusto.png).
- [Consulta do operador aos pedidos reais](../Codigo/evidencias/painel-operador-projeto-real.json).
- [Aprovação fictícia em armazenamento isolado](../Codigo/evidencias/painel-operador-aprovacao-isolada.json).
