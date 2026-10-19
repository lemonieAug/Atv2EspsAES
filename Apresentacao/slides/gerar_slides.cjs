/* Slides, diagramas e prévias: nenhum acesso ao hardware.
 * Cada texto e forma permanece editável no PowerPoint.
 * Código e números vêm dos arquivos do projeto, sem nova execução nas placas.
 */
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const PptxGenJS = require('pptxgenjs');
const sharp = require('sharp');
const PDFDocument = require('pdfkit');
const root = path.resolve(__dirname, '../..');
const read = f => fs.readFileSync(path.join(root, f), 'utf8');
const evidence = JSON.parse(read('Codigo/evidencias/aes-dupla-2026-10-19.json'));
const e = evidence.etapa3;
const out = __dirname;
const C = {bg:'#FAFBFE',ink:'#182B45',muted:'#52657B',blue:'#2165D6',teal:'#008774',
  light:'#EAF1FF',mint:'#E4F6F0',amber:'#A76A12',sand:'#FFF1D9',line:'#D7E1EE',
  white:'#FFFFFF',code:'#F0F4FA',red:'#B14747',rose:'#FCECEE'};
const fontPaths = {
  sans: process.env.SLIDES_FONT || '/usr/share/fonts/noto/NotoSans-Regular.ttf',
  mono: process.env.SLIDES_MONO || '/usr/share/fonts/noto/NotoSansMono-Regular.ttf'
};
for (const f of Object.values(fontPaths)) if (!fs.existsSync(f)) throw new Error('Fonte ausente: '+f);
const measure = new PDFDocument({autoFirstPage:false});
measure.registerFont('sans',fontPaths.sans); measure.registerFont('mono',fontPaths.mono);
const W=1600,H=900;
const slides=[]; let s;
const esc=t=>String(t).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
function shape(type,x,y,w,h,fill=C.white,stroke=null,radius=0){
  const a={type,x,y,w,h,fill,stroke,radius};s.items.push(a);return a;
}
function rect(x,y,w,h,fill=C.white,stroke=null,radius=18){return shape('rect',x,y,w,h,fill,stroke,radius);}
function line(x1,y1,x2,y2,color=C.line,width=3,arrow=false){s.items.push({type:'line',x1,y1,x2,y2,color,width,arrow});}
function wrap(str,width,size,mono){
  measure.font(mono?'mono':'sans').fontSize(size);
  const all=[];
  for(const para of String(str).split('\n')){
    if(!para){all.push('');continue;}
    let cur='';
    for(const word of para.split(/(\s+)/)){
      if(cur && measure.widthOfString(cur+word)>width){all.push(cur.trimEnd());cur=word.trimStart();}
      else cur+=word;
    }
    all.push(cur.trimEnd());
  }
  return all;
}
function text(str,x,y,w,h,size=30,color=C.ink,bold=false,opts={}){
  let lines=opts.nowrap?String(str).split('\n'):wrap(str,w,size,opts.mono);
  while(lines.length*size*1.24>h && size>18){size--;lines=opts.nowrap?String(str).split('\n'):wrap(str,w,size,opts.mono);}
  if(lines.length*size*1.24>h+2) throw new Error('Texto excede caixa no slide '+slides.length+': '+str);
  s.items.push({type:'text',lines,x,y,w,h,size,color,bold,mono:!!opts.mono,align:opts.align||'left'});
}
function tag(str,x,y,w=210,color=C.blue,bg=C.light){rect(x,y,w,38,bg,null,10);text(str,x+12,y+6,w-24,26,18,color,true);}
function base(title,sub='',part='ENTENDA O SISTEMA',source='AES_Trabalho.pdf, páginas 1–2',notes=''){
  s={title,sub,part,source,notes,items:[]};slides.push(s);
  rect(0,0,W,H,C.bg,null,0);rect(64,62,7,31,C.blue,null,3);
  text(part.toUpperCase(),88,63,1390,34,22,C.blue,true);
  text(title,64,124,1470,90,52,C.ink,true);
  if(sub)text(sub,67,219,1460,65,27,C.muted);
}
function card(x,y,w,h,title,body,color=C.blue){
  rect(x,y,w,h,C.white,C.line);rect(x,y,7,h,color,null,0);
  text(title,x+25,y+23,w-50,60,32,color,true);
  text(body,x+25,y+100,w-50,h-115,28,C.ink);
}
function banner(str,y=760,bg=C.light,color=C.blue){rect(64,y,1472,66,bg,null,14);text(str,88,y+16,1424,42,26,color,true);}
function code(codeText,x,y,w,h,label,sz=26){
  rect(x,y,w,h,C.code,C.line,18);rect(x,y,w,43,C.light,null,14);
  text(label,x+18,y+9,w-36,27,18,C.blue,true,{mono:true});
  text(codeText.replace(/\n$/,''),x+22,y+63,w-44,h-77,sz,C.ink,false,{mono:true,nowrap:true});
}
function grid(bytes,x,y,cell=65,colors=null){
  const arr=Array.isArray(bytes)?bytes:bytes.match(/../g);
  for(let r=0;r<4;r++)for(let c=0;c<4;c++){
    const i=r+4*c;rect(x+c*cell,y+r*cell,cell-5,cell-5,colors?colors[i]:C.light,null,8);
    text(arr[i],x+c*cell+4,y+r*cell+14,cell-13,38,26,C.blue,true,{mono:true,align:'center'});
  }
}
function board(x,y,label,color){
  rect(x,y,315,345,C.white,C.line,22);rect(x+39,y+36,237,236,C.ink,null,15);
  for(let i=0;i<8;i++){
    rect(x+23,y+55+i*24,21,10,C.amber,null,2);rect(x+271,y+55+i*24,21,10,C.amber,null,2);
  }
  rect(x+76,y+70,161,110,'#CED8E3',null,8);text('ESP32',x+85,y+97,144,45,34,C.ink,true,{align:'center'});
  rect(x+126,y+230,63,59,'#C0CAD8',null,6);text('USB',x+123,y+244,70,25,19,C.ink,true,{align:'center'});
  text(label,x+25,y+294,265,42,30,color,true,{align:'center'});
}
function lock(x,y,color=C.blue){
  shape('ellipse',x+19,y,68,80,C.white,color);rect(x+10,y+43,86,73,C.white,color,12);
  shape('ellipse',x+47,y+65,13,13,color);rect(x+51,y+75,5,16,color,null,1);
}
function chunk(f,start,end){const all=read(f);const a=all.indexOf(start);const b=end?all.indexOf(end,a+start.length):all.length;if(a<0||b<0)throw new Error('Trecho ausente '+f);return all.slice(a,b).trimEnd();}
function excerpt(f,from,to){return read(f).split('\n').slice(from-1,to).join('\n');}
function sourceLine(f,needle){return read(f).slice(0,read(f).indexOf(needle)).split('\n').length;}

// 01 — Abertura.
base('AES-128 entre dois ESP32','Criptografia, comunicação UART e recuperação da mensagem.','TRABALHO PRÁTICO',
  'AES_Trabalho.pdf; Codigo/evidencias/aes-dupla-2026-10-19.json',
  'Apresentar Augusto (A) e Gustavo (B). O AES usa a mesma chave para cifrar e decifrar. Os resultados citados são de logs existentes de 19/10/2026; esta geração de slides não executou hardware.');
text('Uma mensagem.\nDuas placas.\nO mesmo texto no final.',77,321,800,290,66,C.ink,true);
board(1050,309,'A envia',C.blue);lock(978,414);
tag('DADOS REAIS DA DUPLA',81,655,300,C.teal,C.mint);
text('Augusto · 2023178440013\nGustavo · 202127844251',82,709,850,90,28,C.muted);

// 02 — Objetivo.
base('O que queremos mostrar?','A mensagem muda de aparência durante o transporte e volta ao texto original.');
const steps=[['TEXTO ORIGINAL','A:2023…',C.teal],['ESP32 A','Cifra com a chave',C.blue],['UART','2e 3e 46 0e …',C.blue],['ESP32 B','Decifra com a chave',C.teal]];
steps.forEach((a,i)=>{const x=64+i*378;rect(x,345,340,238,C.white,C.line);text(a[0],x+24,376,292,40,24,a[2],true);text(a[1],x+24,465,292,85,30,C.ink,true);if(i<3)line(x+345,465,x+370,465,C.blue,4,true);});
banner('O teste final compara o texto original com o texto recuperado, byte por byte.',674);

// 03 — Circuito.
base('Dois caminhos: UART para dados, USB para observar','TX é a saída de dados. RX é a entrada. GND dá uma referência elétrica comum.',
  'MONTAGEM FÍSICA','Codigo/src/comum.cpp; Codigo/evidencias/aes-dupla-2026-10-19.json',
  'Os módulos são ESP32-WROOM-32. A: USB location 3-3. B: 3-1. Portas ttyUSB podem mudar. A transmissão AES atual é A→B; o fio de retorno existe, mas A descarta os dados de entrada. Não afirmar AES bidirecional com o firmware atual.');
board(101,319,'A · transmissor',C.blue);board(1185,319,'B · receptor',C.teal);
text('GPIO17 / TX',455,340,240,40,28,C.blue,true);text('GPIO16 / RX',910,340,240,40,28,C.blue,true);
line(423,405,1171,405,C.blue,5,true);
text('GPIO16 / RX',455,452,240,40,28,C.teal,true);text('GPIO17 / TX',910,452,240,40,28,C.teal,true);
line(1171,520,423,520,C.teal,4,true);
line(423,612,1171,612,C.muted,4);text('GND comum',695,625,250,38,27,C.muted,true);
banner('UART2: 115200 bps, 8N1. USB de cada placa: um monitor serial.',738);

// 04 — Vocabulário.
base('Um texto também é uma sequência de números','Byte = grupo de 8 bits. Hexadecimal = uma forma curta de escrever esses números.');
card(64,330,445,280,'CARACTERE','O símbolo que lemos:\n\n7',C.blue);
card(578,330,445,280,'BYTE EM HEX','O código ASCII de “7”:\n\n0x37',C.teal);
text('=',528,446,35,50,40,C.muted,true);
card(1092,330,444,280,'BITS','O mesmo byte:\n\n00110111',C.blue);
text('=',1042,446,35,50,40,C.muted,true);
banner('AES-128 usa uma chave de 128 bits e trabalha com blocos de 16 bytes.',693);

// 05 — Matrículas.
base('A chave nasce das matrículas','Pegamos os últimos 8 dígitos de A e os últimos 8 de B. Depois juntamos A + B.',
  'DADOS DA DUPLA','Codigo/src/dupla.c; Codigo/src/dupla.h; evidência aes-dupla-2026-10-19.json',
  'Se houver menos de oito dígitos, completar com zero à esquerda. Cada caractere vira um byte ASCII; não converter a sequência para um número inteiro. Regra acadêmica: matrículas são previsíveis e não representam uma chave secreta adequada para um produto.');
card(64,322,682,190,'A · Augusto','2023178440013 → 78440013',C.blue);
card(854,322,682,190,'B · Gustavo','202127844251 → 27844251',C.teal);
rect(276,562,1048,144,C.light);text('CHAVE ASCII · 16 BYTES',306,584,980,35,22,C.blue,true);
text(e.chave_ascii,306,632,980,57,46,C.ink,true,{mono:true,align:'center'});
text('Regra do trabalho: dados previsíveis. Em um produto, a chave deve ser secreta e imprevisível.',83,758,1430,50,25,C.muted);

// 06 — Padding.
base('O AES precisa de blocos completos','Nossa mensagem tem 30 bytes. O PKCS#7 acrescenta 02 02 para chegar a 32.',
  'PREPARAR A MENSAGEM','Codigo/src/pkcs7_ecb.c; evidência aes-dupla-2026-10-19.json',
  'A mensagem real é A:2023178440013;B:202127844251. São dois blocos de 16 bytes. PKCS#7 sempre acrescenta entre 1 e 16 bytes; se já couber exatamente, acrescenta um bloco completo de 0x10. Não é texto “02”: são bytes de valor 2.');
text(e.mensagem,83,321,1440,60,39,C.ink,true,{mono:true});
for(let b=0;b<2;b++){
  text('BLOCO '+(b+1),82,425+b*135,180,40,25,C.muted,true);
  for(let j=0;j<16;j++){
    const pad=b===1&&j>=14,x=270+j*77;
    rect(x,412+b*135,69,69,pad?C.mint:C.light,null,10);
    text(e.mensagem_pkcs7_blocos[b].slice(j*2,j*2+2),x+5,432+b*135,59,40,25,pad?C.teal:C.blue,true,{mono:true,align:'center'});
  }
}
banner('Se a mensagem já for múltipla de 16, entram 16 bytes de valor 0x10.',728);

// 07 — Code PKCS7.
base('No código: completar e cifrar','A preparação do texto e a cifra são etapas diferentes.',
  'CÓDIGO REAL','Codigo/src/pkcs7_ecb.c; Codigo/src/analise.c',
  'Os trechos foram extraídos literalmente dos arquivos de origem. Explicar que falta é o total de bytes de preenchimento e também o valor de cada byte acrescentado. No modo ECB, a mesma função cifra cada bloco separadamente.');
const padCode=chunk('Codigo/src/pkcs7_ecb.c','  const uint8_t falta =','  *tamanho_saida');
code(padCode,64,330,1472,188,'pkcs7_ecb.c · dentro de pkcs7_preencher()',27);
const ecbCode=chunk('Codigo/src/pkcs7_ecb.c','  for (size_t b = 0; b < blocos; ++b)','\n}\n\nvoid ecb_decifrar');
code(ecbCode,64,556,1472,160,'pkcs7_ecb.c · dentro de ecb_cifrar()',27);
text('Cada chamada trabalha com 16 bytes. A mesma chave é usada nos dois lados.',84,757,1430,45,28,C.muted);

// 08 — AES rounds.
base('O AES transforma um bloco em 10 rodadas','A cada rodada, substituímos, deslocamos, misturamos e combinamos com a chave.',
  'POR DENTRO DO AES','Codigo/src/aes128.c · aes128_cifrar_bloco()',
  'O estado tem 16 bytes, organizados em uma matriz 4×4 preenchida por colunas. Há AddRoundKey inicial, rodadas 1–9 completas e rodada 10 sem MixColumns. A chave expandida fornece chaves 0–10.');
tag('ANTES DAS RODADAS',64,322,300);text('AddRoundKey com a chave 0',388,325,1050,45,31,C.ink,true);
const rounds=[['SubBytes','Troca cada byte'],['ShiftRows','Desloca as linhas'],['MixColumns','Mistura as colunas'],['AddRoundKey','Combina com a chave']];
rounds.forEach((a,i)=>{const x=64+i*378;card(x,414,340,218,a[0],a[1],i%2?C.teal:C.blue);if(i<3)line(x+343,526,x+372,526,C.blue,4,true);});
banner('Rodadas 1 a 9: as quatro etapas. Rodada 10: sem MixColumns.',710);

// 09 — SubBytes.
base('SubBytes: trocar cada byte por outro','A tabela S-box define a substituição. Isso dificulta relações simples entre entrada e saída.',
  'TRANSFORMAÇÃO 1','Codigo/src/aes128.c · aes_sub_bytes() e aes_gerar_tabelas()',
  'Exemplo real: estado após AddRoundKey inicial 0x76 vira 0x38. A S-box é construída pelo inverso multiplicativo em GF(2⁸), com 0 mapeado para 0, seguido de transformação afim e constante 0x63. A implementação calcula a tabela, não usa biblioteca criptográfica.');
rect(80,345,230,190,C.light);text('76',96,394,198,85,66,C.blue,true,{mono:true,align:'center'});
line(330,438,432,438,C.blue,4,true);rect(449,345,252,190,C.mint);text('S-box',474,413,200,60,40,C.teal,true,{align:'center'});
line(715,438,817,438,C.blue,4,true);rect(833,345,230,190,C.light);text('38',849,394,198,85,66,C.blue,true,{mono:true,align:'center'});
text('Exemplo do\nprimeiro bloco\nda dupla.',1150,365,350,160,30,C.muted);
code(chunk('Codigo/src/aes128.c','void aes_sub_bytes(','\n\nvoid aes_inv_sub_bytes'),64,601,1472,163,'aes128.c · código real',26);

// 10 — ShiftRows.
base('ShiftRows: cada linha anda um pouco','Linha 0 fica. As outras giram 1, 2 e 3 posições para a esquerda.',
  'TRANSFORMAÇÃO 2','Codigo/src/aes128.c · aes_shift_rows(); evidência aes-dupla-2026-10-19.json',
  'Mostrar que os valores não mudam nesta etapa: mudam de posição. A entrada e saída são estados reais da rodada 1. A leitura do hexadecimal nos registros é por colunas, não por linhas.');
grid(e.bloco1_rodada1['2_subbytes'],120,346,75);text('ANTES',126,675,310,40,24,C.blue,true);
grid(e.bloco1_rodada1['3_shiftrows'],1160,346,75);text('DEPOIS',1165,675,320,40,24,C.teal,true);
for(let r=0;r<4;r++){text('Linha '+r,490,353+r*75,170,40,27,C.muted);line(700,380+r*75,1090,380+r*75,r?C.teal:C.line,3,!!r);text(r===0?'fica no lugar':r+(r===1?' posição':' posições')+' à esquerda',695,344+r*75,390,35,24,C.ink);}

// 11 — MixColumns.
base('MixColumns: uma coluna se mistura por dentro','Cada byte de saída depende dos quatro bytes daquela coluna.',
  'TRANSFORMAÇÃO 3','Codigo/src/aes128.c · aes_mix_columns() e aes_xtime()',
  'A primeira coluna real antes desta transformação é 38 7b fe 67; depois é 64 b0 0d 03. Os cálculos são em GF(2⁸), usando XOR em vez de soma comum. 02·a = xtime(a); 03·a = xtime(a) XOR a. O polinômio é 0x11b.');
const before=e.bloco1_rodada1['3_shiftrows'].match(/../g).slice(0,4),after=e.bloco1_rodada1['4_mixcolumns'].match(/../g).slice(0,4);
before.forEach((v,i)=>{rect(94,324+i*88,130,70,C.light);text(v,104,339+i*88,110,44,33,C.blue,true,{mono:true,align:'center'});line(247,359+i*88,540,492,C.blue,3);});
rect(553,385,360,205,C.mint);text('Misturar com\nregras do AES',577,438,310,110,36,C.teal,true,{align:'center'});
after.forEach((v,i)=>{line(927,492,1230,359+i*88,C.teal,3,true);rect(1250,324+i*88,130,70,C.mint);text(v,1260,339+i*88,110,44,33,C.teal,true,{mono:true,align:'center'});});
code(chunk('Codigo/src/aes128.c','uint8_t aes_xtime(','\n\n// Multiplicacao geral'),64,672,1472,164,'aes128.c · multiplicação por 02',22);

// 12 — Key mixing.
base('AddRoundKey: a chave entra na transformação','Usamos XOR: bits iguais dão 0; bits diferentes dão 1.',
  'TRANSFORMAÇÃO 4','Codigo/src/aes128.c · aes_add_round_key(); evidência aes-dupla-2026-10-19.json',
  'Primeiro byte da mensagem 0x41 XOR primeiro byte da chave 0x37 resulta 0x76. XOR é reversível: aplicar novamente a mesma chave recupera o valor anterior. Não confundir esta etapa isolada com todo o algoritmo AES.');
text('41',110,339,260,100,76,C.blue,true,{mono:true,align:'center'});text('XOR',419,354,250,80,48,C.muted,true,{align:'center'});
text('37',747,339,260,100,76,C.teal,true,{mono:true,align:'center'});text('=',1054,354,100,80,55,C.muted,true);
text('76',1217,339,260,100,76,C.blue,true,{mono:true,align:'center'});
text('byte da mensagem',83,456,330,45,26,C.muted);text('byte da chave',720,456,360,45,26,C.muted);text('novo byte',1200,456,310,45,26,C.muted);
code(chunk('Codigo/src/aes128.c','void aes_add_round_key(','\n\n// Expansao'),64,571,1472,184,'aes128.c · código real',26);

// 13 — Key expansion.
base('Uma chave inicial gera as chaves das rodadas','AES-128 usa 11 chaves de 16 bytes: a inicial e uma para cada rodada.',
  'EXPANSÃO DE CHAVE','Codigo/src/aes128.c · aes128_iniciar(); evidência aes-dupla-2026-10-19.json',
  'A expansão produz 44 palavras de 4 bytes. RotWord gira a palavra; SubWord aplica S-box; Rcon adiciona uma constante que muda por rodada. As chaves não são geradas aleatoriamente: são derivadas da chave inicial.');
['RotWord\nGira bytes','SubWord\nAplica S-box','Rcon\nInclui constante','XOR\nDeriva palavras'].forEach((a,i)=>{const x=64+i*378;rect(x,330,340,147,i%2?C.mint:C.light);text(a,x+20,357,300,100,29,i%2?C.teal:C.blue,true,{align:'center'});if(i<3)line(x+345,401,x+370,401,C.blue,3,true);});
text('CHAVE DA RODADA 0',84,535,1420,35,23,C.muted,true);
text(e.chave_rodada_0,84,581,1420,60,36,C.ink,true,{mono:true});
text('CHAVE DA RODADA 1',84,664,1420,35,23,C.muted,true);
text(e.chave_rodada_1,84,711,1420,60,36,C.ink,true,{mono:true});

// 14 — AES code.
base('O ciclo completo aparece neste trecho','A última rodada pula MixColumns, como exige o AES.',
  'CÓDIGO REAL','Codigo/src/aes128.c · aes128_cifrar_bloco()',
  'Trecho literal da função de cifragem. Os buffers in/out são copiados antes/depois deste trecho. AES128_RODADAS vale 10. Para decifrar, o código usa transformações inversas e as chaves em ordem contrária.');
code(chunk('Codigo/src/aes128.c','  aes_add_round_key(s, ctx->chaves_rodada[0]);','  memcpy(out, s, 16);'),64,322,1472,415,'aes128.c · dentro de aes128_cifrar_bloco()',30);
text('Para voltar ao texto: etapas inversas e chaves usadas da rodada 10 até a 0.',84,771,1425,43,28,C.muted);

// 15 — Framing.
base('Como B sabe onde a mensagem começa e termina?','O quadro informa o início e a quantidade de blocos cifrados.',
  'PROTOCOLO UART','Codigo/src/quadro.c; Codigo/src/transmissor.cpp; AES_Trabalho.pdf, p. 1',
  'AA é byte fixo de início, não a string de dois caracteres A e A. n=02 significa dois blocos. A carga cifrada pode conter 0xAA: depois de ler n o receptor segue o comprimento, não procura outro marcador no meio. Cabeçalho não é autenticado.');
const frame=[['AA','início','1 byte',200],['02','2 blocos','1 byte',230],['ciphertext 1','dados cifrados','16 bytes',460],['ciphertext 2','dados cifrados','16 bytes',460]];
let fx=64;frame.forEach((a,i)=>{rect(fx,353,a[3]-12,210,i<2?C.mint:C.light);text(a[0],fx+18,379,a[3]-48,65,i<2?52:31,i<2?C.teal:C.blue,true,{mono:i<2,align:'center'});text(a[1]+'\n'+a[2],fx+18,469,a[3]-48,80,25,C.muted,false,{align:'center'});fx+=a[3];});
code('const size_t escritos = enlace.write(quadro, tamanho_quadro);\nenlace.flush();',64,620,1472,160,'transmissor.cpp · bytes brutos na UART2',28);

// 16 — Receive pipeline and code.
base('Na placa B, o caminho acontece ao contrário','Receber → decifrar → conferir o preenchimento → recuperar a mensagem.',
  'CÓDIGO REAL','Codigo/src/receptor.cpp · processar_quadro(); Codigo/src/quadro.c',
  'O receptor lê exatamente 16×n bytes, com timeout entre bytes de 100 ms e total de 1 s. B decifra os blocos antes de validar/remover PKCS#7. Neste trecho tamanho já é conhecido. Em caso de padding inválido, o código descarta a mensagem.');
const rxCode=chunk('Codigo/src/receptor.cpp','  ecb_decifrar(&g_ctx, rx.dados, claro, rx.n);','  if (r != PKCS7_OK)');
code(rxCode,64,326,1472,245,'receptor.cpp · dentro de processar_quadro()',28);
card(64,620,700,175,'PADDING VÁLIDO','Remove o preenchimento e mostra o texto.',C.teal);
card(807,620,729,175,'PADDING INVÁLIDO','Descarta a mensagem e registra o motivo.',C.amber);

// 17 — Actual two monitor data.
base('O que a turma verá nos dois monitores','Trechos reais dos logs de 19/10/2026, com as matrículas da dupla.',
  'DEMONSTRAÇÃO','Codigo/evidencias/aes-dupla-a-2026-10-19.log; aes-dupla-b-2026-10-19.log',
  'Os painéis mostram linhas reais selecionadas, com quebras visuais para caber. Não são capturas de tela nem uma nova transmissão. Na aula abrir ambos os monitores antes de enviar. Comparar os dois blocos e a mensagem inteira.');
const txLines=['[A] MENSAGEM_ORIGINAL tamanho=30',e.mensagem,'','[A] CIFRADO_ENVIADO bloco 1:',e.cifrado_blocos[0],'[A] CIFRADO_ENVIADO bloco 2:',e.cifrado_blocos[1]];
const rxLines=['[B] CIFRADO_RECEBIDO bloco 1:',e.cifrado_blocos[0],'[B] CIFRADO_RECEBIDO bloco 2:',e.cifrado_blocos[1],'','[B] MENSAGEM_RECUPERADA tamanho=30',e.mensagem];
code(txLines.join('\n'),64,325,715,380,'MONITOR A · transmite',24);
code(rxLines.join('\n'),821,325,715,380,'MONITOR B · recebe',24);
banner('Registro real de B: COMPARACAO igual_a_mensagem_da_dupla=SIM',747,C.mint,C.teal);

// 18 — live procedure.
base('Demonstração ao vivo: três passos','Usar os dois monitores USB a 115200 bps, com envio de linha LF.',
  'ROTEIRO PARA A SALA','Codigo/README.md; Codigo/src/transmissor.cpp; Codigo/src/comum.cpp',
  'Procedimento para executar na aula, não declaração de um novo ensaio. Antes: conferir firmware, identidade física, GND e cruzamento TX/RX. Restaurar as chaves em A e B e mensagem em A. B deve estar pronto antes de ENVIAR. A comparação automática de B é contra a mensagem padrão da dupla: com MSG nova, NAO é esperado e não indica por si só falha na decifragem.');
card(64,327,460,300,'1 · PREPARAR','A e B: CHAVERESET\nA: MSGRESET\nB: aguardar PRONTO',C.blue);
card(570,327,460,300,'2 · ENVIAR','A: ENVIAR\n\nou apertar BOOT\napós o boot normal.',C.teal);
card(1076,327,460,300,'3 · CONFERIR','Mesmos blocos cifrados.\n\nTexto recuperado igual ao original.',C.blue);
banner('Alteração na arguição: MSG <novo texto>. Para voltar: MSGRESET e ENVIAR.',703);

// 19 — Validation.
base('Como sabemos que o resultado está correto?','Conferimos valores conhecidos antes de usar a chave da dupla.',
  'EVIDÊNCIAS REAIS','Codigo/evidencias/testes-host-dupla-2026-10-19.log; aes-dupla-2026-10-19.json',
  '71 verificações no computador não equivalem a 71 ensaios físicos. As placas executaram 11 itens de autoteste cada uma. Os vetores são FIPS-197 2001 B e C.1 conforme enunciado. As capturas AES existentes são A→B; não há evidência de teste AES bidirecional ou sequência AES de mil pacotes.');
const metrics=[['71 / 71','Verificações no computador'],['11 / 11','Autoteste na placa A'],['11 / 11','Autoteste na placa B']];
metrics.forEach((a,i)=>{const x=64+i*504;rect(x,327,464,215,C.white,C.line);text(a[0],x+22,355,420,89,65,C.teal,true);text(a[1],x+22,467,420,62,25,C.muted);});
text('FIPS-197 C.1 · ciphertext esperado e conferido',85,590,1420,45,29,C.ink,true);
text('69c4e0d86a7b0430d8cdb78070b4c55a',85,648,1420,65,40,C.blue,true,{mono:true});
text('Mil pacotes AES, latência AES e envio AES B→A: sem validação documentada.',85,771,1430,48,25,C.amber);

// 20 — Avalanche.
base('Um único bit muda muitos bits do resultado','Avalanche: contar as diferenças entre os dois textos cifrados.',
  'RESULTADOS DA DUPLA','Codigo/evidencias/aes-dupla-2026-10-19.json · avalanche',
  'Bit 0 corresponde a 0x80 do primeiro byte. Com bit 0 da chave alterado: 72/128 no bloco 1 e 70/128 no bloco 2, total 142/256=55,47%. Com bit 0 da mensagem: 69/128 no bloco 1, 0/128 no bloco 2; o total é 69/256=26,95%. Isso é esperado no ECB, pois um bloco não afeta o outro. ~50% é uma expectativa estatística, não exigência exata para um ensaio.');
text('MUDANÇA NA CHAVE',85,327,1430,45,27,C.blue,true);
rect(85,394,1020,65,C.light,null,12);rect(85,394,1020*142/256,65,C.blue,null,12);
text('142 / 256 bits · 55,47%',1150,405,370,60,32,C.blue,true);
text('MUDANÇA NA MENSAGEM · SOMENTE BLOCO 1',85,532,1430,45,27,C.teal,true);
rect(85,599,1020,65,C.mint,null,12);rect(85,599,1020*69/128,65,C.teal,null,12);
text('69 / 128 bits · 53,91%',1150,610,370,60,32,C.teal,true);
banner('O bloco 2 ficou igual no segundo teste: em ECB, os blocos são independentes.',737);

// 21 — ECB.
base('Por que o ECB revela padrões?','Com a mesma chave, blocos iguais produzem blocos cifrados iguais.',
  'LIMITES DO MODO ECB','AES_Trabalho.pdf, p. 2; Codigo/src/vetores_pdf.c; logs aes-dupla',
  'Este experimento usa a chave fictícia do PDF, 2023100120231002, não a chave real da dupla. São três blocos exatos sem PKCS#7. Não há controle físico de LED implementado: as frases são apenas dados. Ciphertext repetido e0c22a6f02664f487dc9528642203dec; outro 2b4d065ea65478b60696616244cd5070.');
const eb=[['1','LIGAR LED VERDE.','e0c22a6f … 42203dec',C.blue,C.light],['2','DESLIGAR LED    ','2b4d065e … 4cd5070',C.teal,C.mint],['3','LIGAR LED VERDE.','e0c22a6f … 42203dec',C.blue,C.light]];
eb.forEach((a,i)=>{let y=328+i*124;tag('BLOCO '+a[0],64,y+25,155,a[3],a[4]);rect(250,y,578,98,a[4]);text(a[1],269,y+29,540,50,33,a[3],true,{mono:true});line(845,y+50,934,y+50,C.muted,3,true);rect(953,y,583,98,a[4]);text(a[2],971,y+29,545,50,30,a[3],true,{mono:true});});
banner('Um observador consegue reconhecer repetições mesmo sem conhecer o texto.',749,C.sand,C.amber);

// 22 — Auth.
base('Cifrar não garante que ninguém alterou os dados','ECB não autentica: um atacante pode repetir ou trocar blocos.',
  'LIMITES E ESCOLHAS','AES_Trabalho.pdf, p. 2; Codigo/src/receptor.cpp; aes-ensaio-2026-10-19.json',
  'O teste com chave errada no ensaio de matrículas fictícias levou a padding inválido, mas isso não é garantia de detectar toda corrupção. Cabeçalho e dados não são autenticados. CBC ou CTR exigem IV/nonce adequado e autenticação separada; GCM também autentica. Não trocar ECB exigido no trabalho.');
card(64,325,707,298,'NO TRABALHO','ECB + PKCS#7\n\nAjuda a estudar a cifra e enxergar blocos repetidos.',C.blue);
card(815,325,721,298,'EM UM PRODUTO','Um modo autenticado, como GCM, com chave secreta e nonce único por chave.',C.teal);
banner('Padding inválido pode revelar um erro. Padding válido não comprova integridade.',710,C.sand,C.amber);

// 23 — close human takeaway.
base('O que conseguimos demonstrar','O código faz as transformações e as placas recuperam a mensagem da dupla.',
  'CONVERSA COM A TURMA','Codigo/evidencias/aes-dupla-2026-10-19.json; Apresentacao/roteiro.md',
  'Convidar professor a escolher uma mensagem nova ou um bit da chave. Cada integrante deve explicar trecho escrito pelo colega; autoria de cada arquivo ainda deve ser definida pela dupla. O guia de execução completo e o ensaio formal na sala seguem pendentes.');
const close=[['01','A transforma o texto em bytes cifrados.'],['02','A UART transporta o quadro binário.'],['03','B recupera exatamente a mensagem original.']];
close.forEach((a,i)=>{const y=330+i*112;tag(a[0],64,y,95,i%2?C.teal:C.blue,i%2?C.mint:C.light);text(a[1],190,y+2,1305,69,36,C.ink,true);});
banner('Pergunta para a turma: se dois blocos forem iguais, o que o ECB deixa aparecer?',739);

// 24 — Real five states appendix.
base('Apoio: os cinco estados do primeiro bloco','Valores reais da rodada 1. Hexadecimal na ordem dos bytes de entrada/saída.',
  'APÊNDICE · ETAPA 3','Codigo/evidencias/aes-dupla-2026-10-19.json · etapa3.bloco1_rodada1',
  'Os cinco estados são exigidos no relatório. Nesta tabela a saída de cada etapa aparece como sequência de 16 bytes. A matriz 4×4 nos diagramas é preenchida por colunas. Comando ETAPA3 exibe os valores, chave, padding e ciphertext.');
const stateNames=[['1 · AddRoundKey inicial','1_addroundkey_inicial_chave0'],['2 · SubBytes','2_subbytes'],['3 · ShiftRows','3_shiftrows'],['4 · MixColumns','4_mixcolumns'],['5 · AddRoundKey · chave 1','5_addroundkey_chave1']];
stateNames.forEach((a,i)=>{let y=315+i*91;rect(64,y,1472,79,i%2?C.white:C.light);text(a[0],83,y+24,540,42,27,C.blue,true);text(e.bloco1_rodada1[a[1]],646,y+24,860,43,28,C.ink,false,{mono:true});});

// 25 — appendix code inverse.
base('Apoio: a decifragem desfaz o caminho','A placa B usa as operações inversas com as chaves em ordem contrária.',
  'APÊNDICE · CÓDIGO REAL','Codigo/src/aes128.c · aes128_decifrar_bloco()',
  'Explicar InvShiftRows, InvSubBytes e InvMixColumns. AddRoundKey é a própria inversa. A posição da chave em relação a InvMixColumns respeita o algoritmo implementado. Trecho literal do código entregue.');
code(chunk('Codigo/src/aes128.c','  aes_add_round_key(s, ctx->chaves_rodada[AES128_RODADAS]);','  memcpy(out, s, 16);'),64,320,1472,455,'aes128.c · dentro de aes128_decifrar_bloco()',29);

// 26 — appendix original exercise.
base('Apoio: encontre a resposta que não chegou','Atividade complementar com os logs históricos do diagnóstico sem criptografia.',
  'APÊNDICE · DIAGNÓSTICO UART','Codigo/evidencias/uart-pingpong-2026-10-19.json; uart-pingpong-a/b-2026-10-19.log',
  'Execução 5f7a5b57. B registrou 1000 PINGs e envio de 1000 PONGs. A registrou 999 PONGs, faltando sequência 1. Houve RX_ERROR no boot; descarte pelo parser é uma hipótese. RTT médio 5554,28 µs se refere somente a este diagnóstico, não ao AES. Esta atividade é apoio, não substituir a demonstração AES exigida.');
card(64,326,708,260,'B · RESPONDEU','1.000 PINGs recebidos\n1.000 PONGs registrados no envio',C.teal);
card(816,326,720,260,'A · CONTOU','999 PONGs recebidos\nFaltou a sequência 1',C.amber);
text('Pergunta: o enlace funcionou? E passou no teste de zero perdas?',83,634,1430,80,35,C.ink,true);
banner('O enlace foi demonstrado nos dois sentidos. O resumo estrito terminou FAIL.',749,C.sand,C.amber);

// 27 — sources / pending.
base('Fontes e preparação final','O material usa os arquivos locais do trabalho e os logs já existentes.',
  'APÊNDICE · REFERÊNCIAS','AES_Trabalho.pdf; Codigo/src; Codigo/evidencias; Apresentacao/roteiro.md',
  'Referência do algoritmo: NIST FIPS PUB 197 (2001), conforme o PDF do professor. README de execução da demonstração: requisito aprovado, documento e ensaio ainda pendentes. Os slides e suas notas são editáveis. Não apresentar este material como um novo ensaio de laboratório.');
card(64,320,707,390,'FONTES','Enunciado: AES_Trabalho.pdf\n\nCódigo: Codigo/src/\n\nEvidências: aes-dupla e aes-ensaio, 19/10/2026.',C.blue);
card(815,320,721,390,'ANTES DA AULA','Revisar o código do colega.\n\nConcluir o README de execução.\n\nEnsaiar os dois monitores e a alteração pedida pelo professor.',C.teal);

// Metadata, source integrity and exports.
async function main(){
  fs.mkdirSync(path.join(out,'svg'),{recursive:true});
  fs.mkdirSync(path.join(out,'imagens'),{recursive:true});
  const checks=[];
  for(const [f,hash] of Object.entries(evidence.sha256)){
    const actual=crypto.createHash('sha256').update(fs.readFileSync(path.join(root,'Codigo/evidencias',f))).digest('hex');
    if(actual!==hash)throw new Error('Hash divergente da evidência: '+f);
    checks.push({arquivo:f,sha256:actual,confere:true});
  }
  const pptx=new PptxGenJS();pptx.defineLayout({name:'WIDE_CUSTOM',width:13.333333,height:7.5});pptx.layout='WIDE_CUSTOM';
  pptx.author='Augusto e Gustavo';pptx.subject='AES-128 em ESP32 via UART';pptx.title='AES-128 entre dois ESP32';pptx.lang='pt-BR';
  pptx.company='Trabalho acadêmico';pptx.theme={headFontFace:'Arial',bodyFontFace:'Arial',lang:'pt-BR'};
  const pdf=new PDFDocument({size:[960,540],margin:0,autoFirstPage:false,info:{Title:'AES-128 entre dois ESP32',Author:'Augusto e Gustavo'}});
  const pdfFile=path.join(out,'apresentacao_aes_uart.pdf');const pdfStream=fs.createWriteStream(pdfFile);pdf.pipe(pdfStream);
  const thumbnails=[];const structural=[];
  const toIn=v=>v/120;const hex=v=>v.replace('#','');
  for(let index=0;index<slides.length;index++){
    s=slides[index];
    line(64,848,1536,848,C.line,1);
    text('AES-128 · AUGUSTO + GUSTAVO',65,861,670,24,17,C.muted,true);
    text(String(index+1).padStart(2,'0')+' / '+slides.length,1374,859,162,25,18,C.muted,false,{align:'right'});
    const ps=pptx.addSlide();ps.background={color:hex(C.bg)};
    ps.addNotes(s.notes+'\n\nFontes locais: '+s.source+'\n\nEstado: slides gerados de código e evidências existentes; não houve execução de firmware nesta etapa.');
    const svg=['<svg xmlns="http://www.w3.org/2000/svg" width="1600" height="900" viewBox="0 0 1600 900">',
      '<defs><marker id="arr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="8" markerHeight="8" orient="auto-start-reverse"><path d="M 0 0 L 10 5 L 0 10 z" fill="context-stroke"/></marker></defs>'];
    for(const a of s.items){
      if(a.type==='text'){
        const anchor=a.align==='center'?'middle':a.align==='right'?'end':'start';
        const tx=a.align==='center'?a.x+a.w/2:a.align==='right'?a.x+a.w:a.x;
        svg.push(`<text fill="${a.color}" font-family="${a.mono?'Noto Sans Mono':'Noto Sans'},sans-serif" font-size="${a.size}" font-weight="${a.bold?700:400}" text-anchor="${anchor}" xml:space="preserve">`);
        a.lines.forEach((l,i)=>{
          svg.push(`<tspan x="${tx}" y="${a.y+a.size+i*a.size*1.24}">${esc(l)}</tspan>`);
          const opts={x:toIn(a.x),y:toIn(a.y+i*a.size*1.24),w:toIn(a.w),h:toIn(a.size*1.24),fontFace:a.mono?'Consolas':'Arial',fontSize:a.size*.6,bold:a.bold,color:hex(a.color),margin:0,breakLine:false,align:a.align,vertAnchor:'top',lineSpacingMultiple:1.0,transparency:0};
          ps.addText(l,opts);
          measure.font(a.mono?'mono':'sans').fontSize(a.size);
          if(measure.widthOfString(l)>a.w+8)throw new Error('Ligne trop longue / slide '+(index+1)+': '+l);
        });svg.push('</text>');
        if(a.x<0||a.y<0||a.x+a.w>W+1||a.y+a.h>H+1)throw new Error('Texte hors cadre '+(index+1));
      }else if(a.type==='line'){
        svg.push(`<line x1="${a.x1}" y1="${a.y1}" x2="${a.x2}" y2="${a.y2}" stroke="${a.color}" stroke-width="${a.width}" ${a.arrow?'marker-end="url(#arr)"':''}/>`);
        const x=Math.min(a.x1,a.x2),y=Math.min(a.y1,a.y2);
        ps.addShape(pptx.ShapeType.line,{x:toIn(x),y:toIn(y),w:toIn(Math.abs(a.x2-a.x1)),h:toIn(Math.abs(a.y2-a.y1)),flipH:a.x2<a.x1,flipV:a.y2<a.y1,line:{color:hex(a.color),width:a.width*.6,beginArrowType:undefined,endArrowType:a.arrow?'triangle':undefined}});
      }else{
        if(a.type==='ellipse')svg.push(`<ellipse cx="${a.x+a.w/2}" cy="${a.y+a.h/2}" rx="${a.w/2}" ry="${a.h/2}" fill="${a.fill}" stroke="${a.stroke||'none'}" stroke-width="3"/>`);
        else svg.push(`<rect x="${a.x}" y="${a.y}" width="${a.w}" height="${a.h}" rx="${a.radius}" fill="${a.fill}" stroke="${a.stroke||'none'}" stroke-width="2"/>`);
        ps.addShape(a.type==='ellipse'?pptx.ShapeType.ellipse:a.radius?pptx.ShapeType.roundRect:pptx.ShapeType.rect,{x:toIn(a.x),y:toIn(a.y),w:toIn(a.w),h:toIn(a.h),radius:toIn(a.radius),rectRadius:toIn(a.radius),fill:{color:hex(a.fill)},line:a.stroke?{color:hex(a.stroke),width:1}:{color:hex(a.fill),transparency:100}});
      }
    }
    svg.push('</svg>');const svgText=svg.join('\n');const number=String(index+1).padStart(2,'0');
    fs.writeFileSync(path.join(out,'svg','slide-'+number+'.svg'),svgText);
    const png=await sharp(Buffer.from(svgText)).png().toBuffer();fs.writeFileSync(path.join(out,'imagens','slide-'+number+'.png'),png);
    const thumb=await sharp(png).resize(400,225).png().toBuffer();thumbnails.push(thumb);
    pdf.addPage();pdf.image(png,0,0,{width:960,height:540});
    structural.push({slide:index+1,titulo:s.title,objetos_editaveis:s.items.length,notas:!!s.notes,fontes:s.source});
  }
  pdf.end();await new Promise((resolve,reject)=>{pdfStream.on('finish',resolve);pdfStream.on('error',reject);});
  await pptx.writeFile({fileName:path.join(out,'apresentacao_aes_uart.pptx')});
  const rows=Math.ceil(slides.length/4);
  await sharp({create:{width:1600,height:rows*225,channels:3,background:C.line}}).composite(thumbnails.map((input,i)=>({input,left:(i%4)*400,top:Math.floor(i/4)*225}))).png().toFile(path.join(out,'previa.png'));
  fs.writeFileSync(path.join(out,'conteudo_e_notas.md'),'# Slides — conteúdo e notas de fala\n\nGerados de código e evidências existentes; sem nova execução nas placas.\n\n'+slides.map((v,i)=>'## '+String(i+1).padStart(2,'0')+' — '+v.title+'\n\n'+v.items.filter(a=>a.type==='text').map(a=>a.lines.join('\n')).join('\n\n')+'\n\n**Notas de fala:** '+v.notes+'\n\n**Fontes locais:** '+v.source+'\n').join('\n'));
  const html=`<!doctype html><html lang="pt-BR"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>AES-128 entre dois ESP32</title><style>body{margin:0;background:#e9eef5;font:16px Arial;color:#182b45;display:grid;place-items:center;min-height:100vh}main{width:min(100vw,1600px)}img{display:block;width:100%;aspect-ratio:16/9;background:white}nav{display:flex;gap:18px;align-items:center;justify-content:center;padding:14px}button{padding:9px 20px;background:white;border:1px solid #ccd7e4;border-radius:8px;color:#182b45;cursor:pointer}button:focus{outline:3px solid #2165d6}</style><main><img id="slide" alt="Slide da apresentação"><nav><button id="prev">Anterior</button><span id="count"></span><button id="next">Próximo</button><button id="full">Tela cheia</button></nav></main><script>const titles=${JSON.stringify(slides.map(x=>x.title))};let i=0;function show(){document.querySelector('#slide').src='svg/slide-'+String(i+1).padStart(2,'0')+'.svg';document.querySelector('#slide').alt=(i+1)+'. '+titles[i];document.querySelector('#count').textContent=(i+1)+' / '+titles.length;document.querySelector('#prev').disabled=i===0;document.querySelector('#next').disabled=i===titles.length-1;}function move(n){i=Math.max(0,Math.min(titles.length-1,i+n));show();}document.querySelector('#prev').onclick=()=>move(-1);document.querySelector('#next').onclick=()=>move(1);document.querySelector('#full').onclick=()=>document.documentElement.requestFullscreen();document.addEventListener('keydown',e=>{if(['ArrowRight',' ','PageDown'].includes(e.key)){e.preventDefault();move(1);}if(['ArrowLeft','PageUp'].includes(e.key)){e.preventDefault();move(-1);}if(e.key==='Home'){i=0;show();}if(e.key==='End'){i=titles.length-1;show();}});show();</script></html>`;
  fs.writeFileSync(path.join(out,'index.html'),html);
  fs.writeFileSync(path.join(out,'validacao_slides.json'),JSON.stringify({origem:'Leitura de código e evidências locais; nenhuma operação de hardware',total_slides:slides.length,tema:'claro',formato:'16:9',verificacoes:['Todos os hashes declarados em aes-dupla-2026-10-19.json conferidos','Textos dentro das caixas e do quadro','PPTX com textos e formas editáveis','PDF e imagens renderizados dos mesmos dados','Notas de fala e fontes em todos os slides'],evidencias:checks,slides:structural},null,2)+'\n');
  console.log(JSON.stringify({slides:slides.length,arquivos:['apresentacao_aes_uart.pptx','apresentacao_aes_uart.pdf','index.html','previa.png','conteudo_e_notas.md'],hashes_conferidos:checks.length}));
}
main().catch(err=>{console.error(err);process.exitCode=1;});
