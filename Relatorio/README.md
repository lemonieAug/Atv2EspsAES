# Relatório

- [relatorio.md](relatorio.md): texto do relatório, legível direto no GitHub.
- [relatorio.pdf](relatorio.pdf): versão para entrega, em A4, com **4 páginas**
  (limite do enunciado).
- [figuras/](figuras): os 7 diagramas em SVG.
- [validacao-laboratorio.md](validacao-laboratorio.md): registro técnico de todos
  os ensaios, inclusive as falhas.

## Como gerar o PDF

Na raiz do projeto:

```sh
python3 Relatorio/ferramentas/gerar_relatorio.py
```

O script ([ferramentas/gerar_relatorio.py](ferramentas/gerar_relatorio.py)) faz
três coisas:

1. Gera as figuras a partir dos dados reais do ensaio
   (`Codigo/evidencias/aes-dupla-2026-10-19.json`), sem copiar números à mão.
2. Monta o PDF a partir de `relatorio.md`, com Cairo, Pango e librsvg via
   PyGObject, já disponíveis no sistema.
3. Conta as páginas no próprio arquivo e falha se passar de 4.

Com `--png PASTA` ele também salva uma prévia de cada página.

Para editar o relatório, altere `relatorio.md` e rode o script de novo. O script
entende o subconjunto de Markdown usado no arquivo: títulos `#`/`##`,
parágrafos, listas `-`, tabelas, citações `>` e imagens `![legenda](figuras/...)`.
