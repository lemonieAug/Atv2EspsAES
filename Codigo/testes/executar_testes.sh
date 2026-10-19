#!/bin/sh
# Compila e executa no computador os testes do codigo C de Codigo/src.
# Uso, a partir da raiz do projeto: sh Codigo/testes/executar_testes.sh
# Execucao no PC; nao substitui a validacao nas placas.
set -eu
raiz=$(cd "$(dirname "$0")/../.." && pwd)
saida="$raiz/Codigo/testes/build"
mkdir -p "$saida"
cc=${CC:-cc}
"$cc" -std=c99 -O2 -Wall -Wextra -Wpedantic -Werror \
  -I "$raiz/Codigo/src" \
  "$raiz/Codigo/testes/teste_host.c" \
  "$raiz/Codigo/src/aes128.c" "$raiz/Codigo/src/pkcs7_ecb.c" \
  "$raiz/Codigo/src/analise.c" "$raiz/Codigo/src/dupla.c" \
  "$raiz/Codigo/src/quadro.c" "$raiz/Codigo/src/vetores_pdf.c" \
  -o "$saida/teste_host"
"$cc" --version | head -n 1
"$saida/teste_host"
