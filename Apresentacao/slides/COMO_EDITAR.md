# Slides — AES-128 entre dois ESP32

Apresentação em português, tema claro e formato 16:9. São 23 slides principais
e 4 de apoio. Textos, diagramas das placas, quadros, setas e trechos de código
são editáveis no PowerPoint. Não foi executada nenhuma operação de hardware
durante a criação deste material.

## Abrir e apresentar

- [PowerPoint editável](apresentacao_aes_uart.pptx): abrir no PowerPoint ou
  importar no LibreOffice Impress. Editar textos, cores, formas e notas do
  apresentador diretamente no arquivo.
- [PDF](apresentacao_aes_uart.pdf): versão pronta para projetar ou compartilhar.
- [Apresentação no navegador](index.html): abrir localmente e navegar pelas
  setas, Espaço ou Page Up/Page Down. Há um botão de tela cheia. Manter as pastas
  `svg/` e o arquivo `index.html` juntos ao copiar para outro computador.
- [Visão geral](previa.png): miniaturas de todos os slides.
- [Conteúdo e notas de fala](conteudo_e_notas.md): texto em Markdown para revisão
  e adaptação da explicação oral.
- `imagens/`: cada slide em PNG, para inserir em outro editor.
- `svg/`: versões vetoriais, para reutilizar diagramas e ilustrações.

O PowerPoint usa Arial e Consolas. O PDF e as imagens foram renderizados com
Noto Sans e Noto Sans Mono. Confira a disposição dos textos após importar em
outro aplicativo. O PDF mantém a composição visual gerada.

## Sequência e conteúdo

| Slides | Assunto |
| --- | --- |
| 1–4 | Objetivo, ligação das placas e vocabulário: byte, bits e hexadecimal |
| 5–7 | Matrículas, chave ASCII e PKCS#7, com código real |
| 8–14 | Rodadas AES, SubBytes, ShiftRows, MixColumns, AddRoundKey e expansão de chave |
| 15–18 | Quadro binário, recepção, logs reais dos dois monitores e sequência da demonstração |
| 19–20 | Vetores/autotestes e medições reais de avalanche |
| 21–23 | Padrões ECB, ausência de autenticação e discussão com a turma |
| 24–27 | Cinco estados reais, decifragem em código, atividade do PONG ausente e fontes |

O PDF do professor não determina quantidade de slides nem duração da fala.
Os apêndices podem ser usados apenas quando houver perguntas.

## Origem dos dados e trechos de código

O material acompanha o [enunciado](../../AES_Trabalho.pdf), o
[roteiro](../roteiro.md) e a implementação atual em
[Codigo/src](../../Codigo/src/). Os trechos mostrados são extraídos literalmente
de `aes128.c`, `pkcs7_ecb.c`, `transmissor.cpp` e `receptor.cpp`.

As matrículas, o ciphertext, os estados e a avalanche vêm de
[aes-dupla-2026-10-19.json](../../Codigo/evidencias/aes-dupla-2026-10-19.json).
Os dois painéis de monitor são transcrições selecionadas dos
[logs de A](../../Codigo/evidencias/aes-dupla-a-2026-10-19.log) e
[logs de B](../../Codigo/evidencias/aes-dupla-b-2026-10-19.log), com quebras
visuais para caber. Não representam uma nova execução nem capturas de tela.
Os desenhos das placas são ilustrações esquemáticas.

Os hashes dos sete arquivos listados no resumo da dupla foram conferidos
durante a geração. A estrutura dos slides e as fontes de cada página estão em
[validacao_slides.json](validacao_slides.json). Todos os slides têm notas de
fala e referências locais no campo de notas do apresentador.

## Cuidados ao explicar os resultados

- O firmware AES atual envia de A para B. O diagnóstico anterior PING/PONG
  demonstrou os dois sentidos sem criptografia. Não apresentar AES B→A como
  resultado já obtido.
- `71/71` refere-se a verificações no computador. `11/11` refere-se ao autoteste
  em cada placa. Não são 1.000 pacotes AES.
- A avalanche da chave foi 142/256 bits, ou 55,47%. Na alteração da mensagem,
  foram 69/128 no primeiro bloco, ou 53,91%; o segundo bloco ficou igual.
  Comparando a mensagem cifrada inteira nesse segundo teste, são 69/256, ou
  26,95%. Os denominadores diferentes estão explicados nas notas.
- O experimento de blocos repetidos usa a chave fictícia do PDF. As frases
  sobre LED são dados do experimento, não comandos para um LED físico.
- ECB não autentica. Padding válido não garante chave correta nem integridade.
- Ao enviar uma mensagem nova com `MSG <texto>`, B compara automaticamente com
  a mensagem padrão das matrículas. Nesse caso, `igual_a_mensagem_da_dupla=NAO`
  é esperado; compare a saída com o novo texto enviado.

## Alterar e gerar novamente

Editar o PPTX é a forma mais direta de ajustar a apresentação. Essas edições
não são importadas de volta para o gerador. Para recriar também o PDF, o HTML
e as imagens, editar [gerar_slides.cjs](gerar_slides.cjs) e executar:

```sh
cd Apresentacao/slides
npm install
npm run gerar
```

O gerador utiliza Node.js, PptxGenJS, Sharp e PDFKit, com versões em
[package.json](package.json). A geração original usou Node.js 26.11.1.
Os caminhos padrão das fontes são `/usr/share/fonts/noto/NotoSans-Regular.ttf`
e `/usr/share/fonts/noto/NotoSansMono-Regular.ttf`. Em outro computador,
configure `SLIDES_FONT` e `SLIDES_MONO` com arquivos TTF compatíveis; instale
também as famílias Noto Sans e Noto Sans Mono para renderizar os SVGs.

Gerar novamente sobrescreve o PPTX, o PDF e as prévias desta pasta. Guarde uma
cópia das edições manuais antes de executar o gerador. Os comandos acima são
somente de geração dos slides e não acessam as portas seriais.

## Pendências da apresentação

- Revisar os slides pela dupla, definir a explicação de trechos escritos pelo
  colega e ensaiar a mudança solicitada pelo professor.
- Validar em ensaio nas duas placas o [guia da demonstração](../README.md),
  escrito em 19/10/2026. O slide 27 ainda lista "Concluir o README de execução";
  leia como "validar o README em ensaio" ou ajuste o slide ao regerar.

Este documento explica a edição e a apresentação dos slides. O guia da
demonstração em hardware está em [Apresentacao/README.md](../README.md).
