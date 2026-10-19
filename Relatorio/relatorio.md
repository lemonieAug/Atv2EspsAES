# AES-128 em dois ESP32 via UART

> Modelo inicial. Substituir os campos PENDENTE e verificar o limite de quatro
> páginas no PDF final. Resultados ainda não foram obtidos.

**Integrantes e matrículas:** PENDENTE  
**Repositório do código:** PENDENTE

## 1. Objetivo e implementação

Descrever brevemente AES-128 do zero, os componentes implementados, o modo ECB,
o PKCS#7 e a comunicação entre as placas. Informar hardware e ferramentas utilizados.

**Ambiente e convenção de representação dos estados:** PENDENTE.

## 2. Validação

| Teste | Resultado obtido | Evidência |
| --- | --- | --- |
| FIPS-197 B: cifragem e decifragem | PENDENTE | PENDENTE |
| FIPS-197 C.1: cifragem e decifragem | PENDENTE | PENDENTE |
| UART entre as duas placas e recuperação da mensagem | PENDENTE | PENDENTE |

## 3. Dados e estados da dupla

**Formação da chave a partir das matrículas:** PENDENTE.

| Registro | Valor hexadecimal |
| --- | --- |
| Chave de 16 bytes | PENDENTE |
| Mensagem com PKCS#7, por bloco | PENDENTE |
| Texto cifrado final, por bloco | PENDENTE |
| Chave de rodada 0 | PENDENTE |
| Chave de rodada 1 | PENDENTE |

Estados do primeiro bloco, na sequência solicitada:

| Transformação | Estado hexadecimal de 16 bytes |
| --- | --- |
| AddRoundKey inicial, chave 0 | PENDENTE |
| SubBytes, rodada 1 | PENDENTE |
| ShiftRows, rodada 1 | PENDENTE |
| MixColumns, rodada 1 | PENDENTE |
| AddRoundKey, chave 1 | PENDENTE |

**Explicação do papel das transformações:** PENDENTE.

## 4. Efeito avalanche

**Entrada original e bit alterado na chave ou mensagem:** PENDENTE.  
**Cifrados original e alterado:** PENDENTE.  
**Bits diferentes / total de bits comparados / percentual:** PENDENTE.  
**Interpretação em relação à expectativa de aproximadamente 50%:** PENDENTE.

## 5. Discussão do modo ECB

Registrar o experimento dos três blocos da página 2 do enunciado. Explicar:

- A repetição do cifrado quando o texto claro e a chave se repetem.
- O que um observador da UART aprende com esse padrão.
- Por que ECB não impede troca ou repetição de blocos.
- Qual alternativa seria utilizada na prática e o papel do IV/nonce e da autenticação.

**Resultados, discussão e limitações do protótipo:** PENDENTE.

## Referências

- Enunciado: `AES_Trabalho.pdf`.
- NIST. FIPS PUB 197 — Advanced Encryption Standard (AES). 2001.
- Outras referências efetivamente consultadas: PENDENTE.
