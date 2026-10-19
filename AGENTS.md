# Orientações para os agentes

## Objetivo e fonte

Preparar e desenvolver o trabalho acadêmico descrito em `AES_Trabalho.pdf`:
AES-128 implementado do zero em C/Arduino, em dois ESP32 conectados pela UART.
O PDF é a fonte dos requisitos. Consulte `README.md` e `TAREFAS.md` antes de trabalhar.
As instruções do usuário prevalecem sobre estas orientações.

## Divisão do trabalho

- `Codigo/`: implementação, configuração de compilação, testes e captura de evidências.
- `Relatorio/`: relatório de até quatro páginas e discussão dos resultados.
- `Apresentacao/`: roteiro, material de apoio e demonstração oral.
- Cada pasta tem seu próprio `AGENTS.md`. Leia-o antes de editar seu conteúdo.
- Esta estrutura permite trabalho independente; ela não inicia agentes automaticamente.
  O coordenador só deve delegar quando houver autorização para isso.
- Quando houver trabalho simultâneo, cada agente deve editar sua pasta designada.
  Mudanças em arquivos da raiz ou em outra pasta devem ser combinadas com o coordenador.
- Não sobrescreva alterações de outros agentes. Informe dependências e impedimentos.

## Regras comuns

- Escreva a documentação em português e use caminhos relativos nos links internos.
- Não altere o PDF original.
- Não invente matrículas, medições, estados AES ou demonstrações realizadas.
  Identifique exemplos fictícios e mantenha resultados pendentes como `PENDENTE`.
- Não apresente testes em computador como validação nas placas físicas.
- Não use bibliotecas criptográficas no código entregue. mbedTLS serve somente
  como referência externa de conferência, fora da implementação entregue.
- Não configure hardware, pinos ou dependências com base em suposições silenciosas.
- Não publique no GitHub nem envie material a terceiros sem instrução do usuário.
- Atualize apenas tarefas efetivamente concluídas em `TAREFAS.md`, mediante coordenação.

## Integração dos resultados

O responsável por `Codigo/` deve documentar os comandos de compilação/teste e
depositar resultados reais em `Codigo/evidencias/`, com configuração e origem.
Relatório e apresentação devem referenciar essas evidências e usar os mesmos
dados, convenções de representação e resultados.

Ao encerrar uma tarefa, informe arquivos alterados, validações executadas e
pendências. A entrega só está completa após a validação nas duas placas reais.
