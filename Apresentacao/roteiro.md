# Roteiro da apresentação

> Material inicial; preencher com os dados reais e ensaiar nas duas placas.
> A duração da apresentação não é especificada no enunciado.

## Sequência sugerida

1. Apresentar os integrantes, matrículas e objetivo do sistema.
2. Mostrar como as matrículas formam a chave ASCII e a mensagem com PKCS#7.
3. Mostrar a validação com os dois vetores FIPS-197.
4. Explicar expansão de chave e transformações usando os cinco estados do bloco 1.
5. Demonstrar o quadro binário pela UART em dois monitores USB/serial lado a lado,
   comparando mensagem original, ciphertext enviado/recebido e mensagem recuperada.
6. Apresentar o experimento avalanche e interpretar a contagem de bits.
7. Mostrar blocos repetidos em ECB, discutir troca/repetição e alternativas.
8. Explicar trechos do código do colega e executar a alteração proposta na arguição.

## Demonstração planejada em dois monitores

**Estado:** planejamento aprovado pelo usuário; implementação e execução
PENDENTES. Esta apresentação detalha a demonstração ao vivo prevista no PDF.

| Monitor USB/serial de A — transmissor | Monitor USB/serial de B — receptor |
| --- | --- |
| Mensagem `A:<matrícula A>;B:<matrícula B>` com os dados reais | Ciphertext efetivamente recebido, em hexadecimal por bloco |
| Mensagem com PKCS#7, em hexadecimal por bloco | Resultado da validação e remoção do PKCS#7 |
| Ciphertext enviado, em hexadecimal por bloco | Mensagem original recuperada e seu comprimento |

O firmware deverá executar AES-128 do zero em C/Arduino nas placas, com ECB e
PKCS#7. O enlace UART2 continuará em 115200 bps, 8N1 e transportará bytes brutos
no formato `0xAA | n (1 byte) | 16 × n bytes cifrados`. Os logs didáticos ficarão
na USB/UART0; sua representação hexadecimal não altera o quadro do enunciado.
Os monitores exibirão registros dos firmwares, não uma captura elétrica
independente da UART2. Um visualizador Python está fora deste escopo aprovado.

Sequência prevista para a aula:

1. Identificar A/B e abrir seus monitores lado a lado antes do envio.
2. Mostrar em A a mensagem da dupla e os bytes de preenchimento.
3. Iniciar a transmissão pelo procedimento que vier a ser implementado e
   documentado; o mecanismo de início/repetição da demonstração AES é PENDENTE.
4. Comparar todos os bytes do ciphertext enviado por A com os recebidos por B.
5. Comparar o comprimento e todos os bytes da mensagem recuperada com a original.
6. Salvar os logs dos dois lados como evidência do mesmo ensaio.

Critério de sucesso: quadro completo recebido dentro do limite definido,
ciphertexts iguais, padding válido e recuperação exata da mensagem da dupla.
Registrar falhas e impedimentos quando ocorrerem. Padding válido não comprova
integridade: ECB/PKCS#7 não fornece autenticação. A igualdade será conferida com
a entrada conhecida do ensaio. A demonstração permanece PENDENTE até haver
evidência real nas duas placas.

## Requisito de README para executar a demonstração

Entregar `Apresentacao/README.md` em português, com links relativos e um passo
a passo que a dupla consiga seguir na sala. É um requisito adicional aprovado
pelo usuário, não um formato de documento imposto pelo PDF.

O guia deverá conter:

- Pré-requisitos e versões utilizadas, arquivos necessários e configuração das
  matrículas reais; distinguir os exemplos fictícios dos dados da dupla.
- Ligações GPIO17 de A → GPIO16 de B, GPIO17 de B → GPIO16 de A e GND comum;
  identificação física de A/B e revalidação das portas após reconexão USB.
- Procedimentos reproduzíveis de compilação e gravação individual de `esp_a`
  e `esp_b` via PlatformIO MCP, respeitando as aprovações de hardware.
- Abertura e encerramento dos dois monitores, identificação de cada janela,
  baud rate e tratamento de portas ocupadas ou sem permissão.
- Ordem de preparação, início e repetição da transmissão, usando somente
  comandos efetivamente implementados e validados no firmware final.
- Leitura dos campos exibidos, critérios de sucesso acima e exemplos de saída
  obtidos em execução real, com referência às evidências.
- Procedimento de captura dos logs em `Codigo/evidencias/` e diagnóstico de
  ausência de dados, timeout, quadro incompleto ou padding inválido.

**Aceitação do README:** executar um ensaio seguindo o guia nas duas placas e
registrar configuração, resultado e evidências. Redação e validação do guia
permanecem PENDENTES; esta etapa autoriza somente seu planejamento.

## Responsabilidades da dupla

| Participante | Trecho escrito pelo colega que irá explicar | Preparação |
| --- | --- | --- |
| A — PENDENTE | PENDENTE | PENDENTE |
| B — PENDENTE | PENDENTE | PENDENTE |

## Preparação da demonstração

- [ ] Conferir firmware correto em cada placa e dados reais da dupla.
- [ ] Conferir TX→RX, RX→TX, GND e os pinos documentados.
- [ ] Conferir UART2 a 115200 bps, 8N1 e monitores seriais disponíveis.
- [ ] Demonstrar transmissão de bytes brutos e recuperação da mensagem original.
- [ ] Comparar ciphertext enviado/recebido e mensagem original/recuperada nos
      dois monitores, preservando os logs do mesmo ensaio.
- [ ] Concluir e validar o README de execução da demonstração nas duas placas.
- [ ] Localizar no código as transformações, expansão de chave e protocolo.
- [ ] Ensaiar mensagem de tamanho diferente e efeito do padding.
- [ ] Ensaiar inversão de um bit da chave ou mensagem e comparação dos cifrados.
- [ ] Ter os resultados e configurações de execução acessíveis para conferência.

## Perguntas para ensaio

- Como a chave é construída e por que ASCII não equivale ao valor numérico do dígito?
- Por que PKCS#7 acrescenta um bloco quando a entrada já é múltipla de 16?
- O que cada transformação do AES faz? De onde vem a S-box?
- Como funciona a multiplicação em GF(2⁸)?
- Como o receptor identifica e lê um quadro? O que acontece se faltarem bytes?
- Por que blocos iguais geram cifrados iguais em ECB?
- O que um observador aprende? O que acontece se trocar ou repetir blocos?
- Qual modo seria usado na prática e por quê?
- Como interpretar um percentual de avalanche diferente de exatamente 50%?

## Registro do ensaio

**Data, participantes e configuração:** PENDENTE.  
**Demonstrações concluídas e evidências:** PENDENTE.  
**Ajustes necessários antes da apresentação:** PENDENTE.
