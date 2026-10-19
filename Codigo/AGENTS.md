# Agente responsável pelo código

Leia também `../AGENTS.md` e o enunciado original.

- Trabalhe dentro de `Codigo/`; combine alterações compartilhadas com o coordenador.
- Implemente o AES-128 em C/Arduino do zero, sem bibliotecas criptográficas.
- Mantenha a lógica AES independente de UART/Arduino quando possível, para
  permitir validação no computador e uso da mesma implementação nas placas.
- Documente a ordem dos bytes no estado AES e nos registros em hexadecimal.
- Valide primeiro os vetores em `testes/vetores.json`; não trate a transcrição
  desses dados como uma execução de teste.
- Configure a plataforma apenas após conhecer o modelo das placas e a ferramenta.
- Use UART2 a 115200 bps, 8N1; envie bytes brutos, não texto hexadecimal.
- Documente limites de blocos, tamanho dos buffers, tempo limite e tratamento
  de quadros incompletos e padding inválido.
- Não misture logs de depuração com a UART usada pelo protocolo.
- Registre evidências reais e suficientes para os outros agentes em `evidencias/`.
- Documente a variante do experimento avalanche: dado alterado, bit invertido,
  cifrados comparados, quantidade de bits diferentes e total comparado.
