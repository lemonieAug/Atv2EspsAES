#!/usr/bin/env python3
"""Gera as figuras SVG e o PDF do relatorio a partir de relatorio.md.

Uso, na raiz do projeto:
    python3 Relatorio/ferramentas/gerar_relatorio.py [--png DIR]

As figuras usam os valores reais de Codigo/evidencias/aes-dupla-2026-10-19.json,
sem copiar numeros a mao. O PDF sai em Relatorio/relatorio.pdf, em A4, e o
script falha se passar de 4 paginas (limite do enunciado). Depende apenas de
Cairo, Pango e librsvg via PyGObject, ja disponiveis no sistema.
"""

import argparse
import html
import json
import re
import sys
import zlib
from pathlib import Path

import cairo
import gi

gi.require_version("Pango", "1.0")
gi.require_version("PangoCairo", "1.0")
gi.require_version("Rsvg", "2.0")
from gi.repository import Pango, PangoCairo, Rsvg  # noqa: E402

RAIZ = Path(__file__).resolve().parents[2]
REL = RAIZ / "Relatorio"
FIG = REL / "figuras"
EVID = RAIZ / "Codigo" / "evidencias" / "aes-dupla-2026-10-19.json"
LIMITE_PAGINAS = 4

# Paleta comum as figuras e ao PDF.
AZUL, AZUL_CLARO = "#1d4ed8", "#eff6ff"
VERDE, VERDE_CLARO = "#047857", "#ecfdf5"
ROXO, ROXO_CLARO = "#6d28d9", "#ede9fe"
AMBAR, AMBAR_CLARO = "#b45309", "#fef3c7"
CINZA, CINZA_CLARO, TEXTO = "#6b7280", "#f3f4f6", "#111827"
CORES_COLUNA = ["#dbeafe", "#dcfce7", "#fef3c7", "#fce7f3"]
FONTE = "Noto Sans, Arial, sans-serif"
MONO = "Noto Sans Mono, DejaVu Sans Mono, monospace"


# ---------------------------------------------------------------- figuras SVG
class Svg:
    def __init__(self, largura, altura):
        self.w, self.h = largura, altura
        self.partes = []

    def add(self, s):
        self.partes.append(s)

    def rect(self, x, y, w, h, fill="#fff", stroke=None, r=6, sw=1.2, dash=None):
        extra = f' stroke="{stroke}" stroke-width="{sw}"' if stroke else ""
        if dash:
            extra += f' stroke-dasharray="{dash}"'
        self.add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{fill}"{extra}/>')

    def text(self, x, y, s, size=12, anchor="start", weight="normal", color=TEXTO,
             mono=False, italic=False):
        fam = MONO if mono else FONTE
        st = ' font-style="italic"' if italic else ""
        self.add(f'<text x="{x}" y="{y}" font-family="{fam}" font-size="{size}" '
                 f'text-anchor="{anchor}" font-weight="{weight}" fill="{color}"{st}>'
                 f'{html.escape(s)}</text>')

    def seta(self, x1, y1, x2, y2, color=CINZA, sw=1.6, dash=None):
        d = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{color}" '
                 f'stroke-width="{sw}" marker-end="url(#ponta)"{d}/>')

    def caminho(self, d, color=CINZA, sw=1.6, dash=None):
        ds = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(f'<path d="{d}" fill="none" stroke="{color}" stroke-width="{sw}" '
                 f'marker-end="url(#ponta)"{ds}/>')

    def chip(self, x, y, w, h, linhas, fill, stroke):
        self.rect(x, y, w, h, fill=fill, stroke=stroke)
        n = len(linhas)
        base = y + h / 2 - (n - 1) * 8 + 4
        for i, (s, peso, tam) in enumerate(linhas):
            self.text(x + w / 2, base + i * 16, s, size=tam, anchor="middle", weight=peso)

    def salvar(self, caminho):
        defs = ('<defs><marker id="ponta" viewBox="0 0 10 10" refX="9" refY="5" '
                'markerWidth="7" markerHeight="7" orient="auto-start-reverse">'
                f'<path d="M0,0 L10,5 L0,10 z" fill="{CINZA}"/></marker></defs>')
        corpo = "\n".join(self.partes)
        caminho.write_text(
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{self.w}" height="{self.h}" '
            f'viewBox="0 0 {self.w} {self.h}">\n{defs}\n'
            f'<rect width="{self.w}" height="{self.h}" fill="#ffffff"/>\n{corpo}\n</svg>\n',
            encoding="utf-8")


def fig1_visao_geral():
    s = Svg(760, 252)
    pos = [24, 170, 316, 462, 608]
    s.rect(10, 8, 740, 104, fill=AZUL_CLARO, stroke=AZUL, r=10)
    s.text(24, 30, "Placa A — Augusto (transmissor)", size=13, weight="bold", color=AZUL)
    passos_a = [("Matrículas", "A e B"), ("Chave + mensagem", "16 bytes · 30 bytes"),
                ("PKCS#7", "completa 2 blocos"), ("AES-128 (ECB)", "cifra cada bloco"),
                ("Quadro", "AA | n | blocos")]
    for i, (a, b) in enumerate(passos_a):
        s.chip(pos[i], 44, 128, 54, [(a, "bold", 12), (b, "normal", 10.5)], "#fff", AZUL)
        if i:
            s.seta(pos[i - 1] + 128, 71, pos[i] - 2, 71)
    s.rect(10, 140, 740, 104, fill=VERDE_CLARO, stroke=VERDE, r=10)
    s.text(24, 236, "Placa B — Gustavo (receptor)", size=13, weight="bold", color=VERDE)
    passos_b = [("Sincroniza", "procura o 0xAA"), ("Lê o quadro", "n e 16·n bytes"),
                ("AES-128 (ECB)", "decifra cada bloco"), ("Remove PKCS#7", "e confere"),
                ("Mostra o texto", "na USB (monitor)")]
    for i, (a, b) in enumerate(passos_b):
        x = pos[4 - i]
        s.chip(x, 150, 128, 54, [(a, "bold", 12), (b, "normal", 10.5)], "#fff", VERDE)
        if i:  # B percorre os passos da direita para a esquerda
            s.seta(pos[5 - i], 177, x + 130, 177)
    s.seta(672, 98, 672, 148, color=TEXTO, sw=2)
    s.text(660, 122, "3 fios: TX17→RX16, RX16←TX17, GND", size=10.5, anchor="end")
    s.text(660, 135, "UART2 · 115200 bps · só bytes ilegíveis passam", size=10.5, anchor="end",
           color=CINZA)
    return s


def celulas(s, x, y, textos, w=20, h=24, fills=None, mono=True, size=12, sub=None):
    for i, t in enumerate(textos):
        f = fills[i] if fills else "#fff"
        s.rect(x + i * w, y, w, h, fill=f, stroke="#9ca3af", r=0, sw=0.8)
        s.text(x + i * w + w / 2, y + h / 2 + 4.5, t, size=size, anchor="middle", mono=mono)
        if sub:
            s.text(x + i * w + w / 2, y + h + 11, sub[i], size=8.5, anchor="middle", mono=True,
                   color=CINZA)


def fig2_chave_mensagem(d):
    s = Svg(760, 262)
    ma, mb = d["dupla"]["A"]["matricula"], d["dupla"]["B"]["matricula"]
    for linha, (rot, m) in enumerate([("Matrícula A", ma), ("Matrícula B", mb)]):
        y = 18 + linha * 40
        s.text(10, y + 17, rot, size=12, weight="bold")
        fills = ["#ffffff"] * (len(m) - 8) + ["#c4b5fd"] * 8
        celulas(s, 100, y, list(m), fills=fills)
    s.text(100, 104, "em roxo: os 8 últimos dígitos de cada matrícula", size=10, color=ROXO)
    s.seta(372, 48, 408, 48)
    chave = d["etapa3"]["chave_ascii"]
    s.text(416, 22, "Chave: 8 de A + 8 de B, cada dígito em ASCII", size=11.5, weight="bold",
           color=ROXO)
    celulas(s, 416, 32, list(chave), fills=["#c4b5fd"] * 16,
            sub=[f"{ord(c):02x}" for c in chave])
    s.text(416, 92, "16 bytes = 128 bits  ('7' → 0x37, '8' → 0x38, ...)", size=10, color=CINZA)
    msg = d["etapa3"]["mensagem"].encode()
    pad = bytes([2, 2])
    s.text(10, 140, f"Mensagem ({len(msg)} bytes) + PKCS#7 ({len(pad)} bytes de valor 0x02) "
           "= 32 bytes = 2 blocos de 16", size=12, weight="bold")
    total = msg + pad
    for b in range(2):
        y = 154 + b * 52
        s.text(10, y + 17, f"Bloco {b + 1}", size=12, weight="bold")
        blk = total[16 * b:16 * b + 16]
        rot = [chr(c) if 32 <= c < 127 else "02" for c in blk]
        fills = [AZUL_CLARO if 32 <= c < 127 else "#fcd34d" for c in blk]
        celulas(s, 100, y, rot, w=40, fills=fills, sub=[f"{c:02x}" for c in blk])
    s.text(100, 256, "em amarelo: bytes de preenchimento (faltavam 2, então entram dois bytes 02)",
           size=10, color=AMBAR)
    return s


def fig3_rodadas():
    s = Svg(760, 206)
    s.chip(8, 26, 96, 60, [("Bloco claro", "bold", 11.5), ("16 bytes", "normal", 10.5)],
           CINZA_CLARO, CINZA)
    s.seta(104, 56, 120, 56)
    s.chip(122, 26, 96, 60, [("AddRoundKey", "bold", 11), ("chave 0", "normal", 10.5)],
           ROXO_CLARO, ROXO)
    s.seta(218, 56, 234, 56)
    s.rect(236, 10, 330, 92, fill=AZUL_CLARO, stroke=AZUL, r=10)
    s.text(401, 28, "Rodadas 1 a 9 (o mesmo ciclo, 9 vezes)", size=11.5, anchor="middle",
           weight="bold", color=AZUL)
    nomes = [("SubBytes", "trocar"), ("ShiftRows", "deslocar"), ("MixColumns", "misturar"),
             ("AddRoundKey", "chave i")]
    for i, (a, b) in enumerate(nomes):
        x = 244 + i * 80
        fill = ROXO_CLARO if i == 3 else "#fff"
        s.chip(x, 38, 74, 52, [(a, "bold", 9), (b, "normal", 9.5)], fill, AZUL)
        if i:
            s.seta(x - 6, 64, x - 1, 64)
    s.caminho("M 520 92 C 520 112, 282 112, 282 93", color=AZUL)
    s.text(401, 122, "repete", size=9, anchor="middle", color=AZUL)
    s.seta(566, 56, 582, 56)
    s.rect(584, 10, 110, 92, fill=AZUL_CLARO, stroke=AZUL, r=10)
    s.text(639, 28, "Rodada 10", size=11.5, anchor="middle", weight="bold", color=AZUL)
    for i, t in enumerate(["SubBytes", "ShiftRows", "AddRoundKey", "(sem MixColumns)"]):
        s.text(639, 50 + i * 15, t, size=10.5 if i < 3 else 9.5, anchor="middle",
               color=TEXTO if i < 3 else CINZA)
    s.seta(694, 56, 708, 56)
    s.chip(710, 26, 44, 60, [("Cifrado", "bold", 9.5)], CINZA_CLARO, CINZA)
    s.rect(122, 132, 572, 40, fill=ROXO_CLARO, stroke=ROXO, r=8)
    s.text(408, 150, "Expansão de chave: 16 bytes → 11 chaves de rodada (0 a 10)", size=11.5,
           anchor="middle", weight="bold", color=ROXO)
    s.text(408, 165, "RotWord (girar) · SubWord (S-box) · XOR com Rcon (constante da rodada)",
           size=10, anchor="middle", color=ROXO)
    for x in (170, 520, 670):
        s.seta(x, 132, x, 98 if x != 170 else 88, color=ROXO, dash="3 3")
    s.text(380, 196, "Decifrar: os passos inversos, na ordem contrária, com as chaves de 10 até 0.",
           size=10.5, anchor="middle", color=CINZA, italic=True)
    return s


def fig4_estados(d):
    e = d["etapa3"]["bloco1_rodada1"]
    ordem = [("Entrada", "bloco 1", e["entrada"]),
             ("(1) AddRoundKey", "chave 0", e["1_addroundkey_inicial_chave0"]),
             ("(2) SubBytes", "trocar", e["2_subbytes"]),
             ("(3) ShiftRows", "deslocar", e["3_shiftrows"]),
             ("(4) MixColumns", "misturar", e["4_mixcolumns"]),
             ("(5) AddRoundKey", "chave 1", e["5_addroundkey_chave1"])]
    s = Svg(760, 186)
    cel = 26
    for g, (t1, t2, hx) in enumerate(ordem):
        x0 = 10 + g * 126
        s.text(x0 + 52, 14, t1, size=10.5, anchor="middle", weight="bold")
        s.text(x0 + 52, 28, t2, size=9.5, anchor="middle", color=CINZA)
        b = bytes.fromhex(hx)
        for c in range(4):
            for r in range(4):
                if g == 2:
                    fill = CORES_COLUNA[c]
                elif g == 3:
                    fill = CORES_COLUNA[(c + r) % 4]  # ShiftRows: veio da coluna (c + r) mod 4
                elif g in (1, 5):
                    fill = ROXO_CLARO
                else:
                    fill = "#fff"
                x, y = x0 + c * cel, 36 + r * cel
                s.rect(x, y, cel, cel, fill=fill, stroke="#9ca3af", r=0, sw=0.8)
                s.text(x + cel / 2, y + cel / 2 + 4, f"{b[r + 4 * c]:02x}", size=10.5,
                       anchor="middle", mono=True)
        if g:
            s.seta(x0 - 19, 88, x0 - 3, 88)
    s.text(10, 160, "Cada tabela é o estado 4×4: o byte i do bloco fica na linha i mod 4, coluna i div 4.",
           size=10, color=CINZA)
    s.text(10, 176, "Cores em (2) e (3): coluna de origem de cada byte; o ShiftRows gira a linha r em r casas.",
           size=10, color=CINZA)
    return s


def fig5_quadro(d):
    cif = d["etapa3"]["cifrado_blocos"]
    s = Svg(760, 232)
    partes = [(30, 50, "AA", "marcador", AMBAR_CLARO, AMBAR),
              (80, 50, "02", "n = 2", AMBAR_CLARO, AMBAR),
              (130, 300, cif[0], "bloco cifrado 1 (16 bytes)", AZUL_CLARO, AZUL),
              (430, 300, cif[1], "bloco cifrado 2 (16 bytes)", AZUL_CLARO, AZUL)]
    for x, w, t, rot, fill, cor in partes:
        s.rect(x, 18, w, 34, fill=fill, stroke=cor, r=0)
        s.text(x + w / 2, 40, t, size=11 if w > 60 else 13, anchor="middle", mono=True,
               weight="bold" if w < 60 else "normal")
        s.text(x + w / 2, 68, rot, size=10.5, anchor="middle", color=cor)
    s.text(380, 88, "34 bytes no total, enviados crus pela UART2 (não como texto hexadecimal)",
           size=10.5, anchor="middle", color=CINZA, italic=True)
    estados = [(40, 170, "Procurando 0xAA"), (300, 120, "Lendo n"), (520, 190, "Lendo 16·n bytes")]
    for x, w, t in estados:
        s.rect(x, 116, w, 42, fill=VERDE_CLARO, stroke=VERDE, r=21)
        s.text(x + w / 2, 142, t, size=12, anchor="middle", weight="bold", color=VERDE)
    s.seta(210, 137, 298, 137, color=VERDE)
    s.text(254, 130, "chegou 0xAA", size=10, anchor="middle")
    s.seta(420, 137, 518, 137, color=VERDE)
    s.text(469, 130, "n ≥ 1", size=10, anchor="middle")
    s.caminho("M 615 158 L 615 196 L 125 196 L 125 160", color=VERDE)
    s.text(370, 190, "completou → decifra, confere o PKCS#7 e mostra o texto", size=10.5,
           anchor="middle", weight="bold")
    s.text(370, 214, "também volta ao início se n = 0 ou se o tempo esgotar "
           "(100 ms sem bytes ou 1 s no total): quadro descartado", size=10, anchor="middle",
           color=CINZA)
    s.text(40, 110, "outros bytes: ignorados", size=9.5, color=CINZA)
    return s


def fig6_avalanche(d):
    def bits(txt):
        return int(txt.split("/")[0])
    av = d["avalanche"]
    linhas = [("Chave, bit 0 · bloco 1", bits(av["chave_bit0"]["bloco1"]), ROXO),
              ("Chave, bit 0 · bloco 2", bits(av["chave_bit0"]["bloco2"]), ROXO),
              ("Mensagem, bit 0 · bloco 1", bits(av["mensagem_bit0"]["bloco1"]), AZUL),
              ("Mensagem, bit 0 · bloco 2", bits(av["mensagem_bit0"]["bloco2"]), AZUL)]
    s = Svg(760, 176)
    x0, larg = 220, 470
    meio = x0 + larg / 2
    for i, (rot, n, cor) in enumerate(linhas):
        y = 22 + i * 32
        s.text(x0 - 10, y + 15, rot, size=11.5, anchor="end")
        s.rect(x0, y, larg, 22, fill=CINZA_CLARO, r=3)
        if n:
            s.rect(x0, y, larg * n / 128, 22, fill=cor, r=3)
        pct = 100 * n / 128
        rotulo = f"{n} de 128 bits ({pct:.1f}%)".replace(".", ",")
        s.text(x0 + larg * n / 128 + 6, y + 15, rotulo, size=10.5, weight="bold")
        if n == 0:
            s.text(meio + 8, y + 15, "← no ECB o bloco 2 não muda", size=10.5, color=AZUL)
    s.add(f'<line x1="{meio}" y1="14" x2="{meio}" y2="150" stroke="{TEXTO}" stroke-width="1.2" '
          'stroke-dasharray="4 3"/>')
    s.text(meio, 11, "metade = 64 bits (esperado ≈ 50%)", size=10, anchor="middle")
    for v in (0, 32, 64, 96, 128):
        x = x0 + larg * v / 128
        s.text(x, 166, str(v), size=9.5, anchor="middle", color=CINZA)
    s.text(x0 - 10, 166, "bits do cifrado alterados:", size=9.5, anchor="end", color=CINZA)
    return s


def fig7_ecb():
    s = Svg(760, 200)
    claros = ["LIGAR LED VERDE.", "DESLIGAR LED    ", "LIGAR LED VERDE."]
    cifs = ["e0c22a6f02664f487dc9528642203dec", "2b4d065ea65478b60696616244cd5070",
            "e0c22a6f02664f487dc9528642203dec"]
    s.text(120, 16, "Texto claro", size=12, anchor="middle", weight="bold")
    s.text(560, 16, "Texto cifrado (mesma chave, modo ECB)", size=12, anchor="middle",
           weight="bold")
    for i in range(3):
        y = 26 + i * 48
        igual = i != 1
        fill, cor = (AMBAR_CLARO, AMBAR) if igual else (CINZA_CLARO, CINZA)
        s.rect(20, y, 200, 36, fill=fill, stroke=cor, r=4)
        s.text(30, y + 23, f"{i + 1}", size=11, weight="bold", color=cor)
        s.text(126, y + 23, f'"{claros[i]}"', size=11.5, anchor="middle", mono=True)
        s.seta(222, y + 18, 376, y + 18)
        s.text(299, y + 13, "AES", size=10, anchor="middle", color=CINZA)
        s.rect(380, y, 360, 36, fill=fill, stroke=cor, r=4)
        s.text(560, y + 23, cifs[i], size=11.5, anchor="middle", mono=True)
    s.text(380, 194, "Blocos 1 e 3 iguais → cifrados iguais: quem escuta a UART percebe a "
           "repetição sem saber a chave.", size=10.5, anchor="middle", color=AMBAR,
           weight="bold")
    return s


def gerar_figuras():
    dados = json.loads(EVID.read_text(encoding="utf-8"))
    FIG.mkdir(exist_ok=True)
    figuras = {
        "fig1-visao-geral.svg": fig1_visao_geral(),
        "fig2-chave-mensagem.svg": fig2_chave_mensagem(dados),
        "fig3-rodadas.svg": fig3_rodadas(),
        "fig4-estados.svg": fig4_estados(dados),
        "fig5-quadro.svg": fig5_quadro(dados),
        "fig6-avalanche.svg": fig6_avalanche(dados),
        "fig7-ecb.svg": fig7_ecb(),
    }
    for nome, svg in figuras.items():
        svg.salvar(FIG / nome)
    return list(figuras)


# ------------------------------------------------------- markdown -> blocos
def inline(texto):
    """Converte o markdown em linha usado no relatorio para marcacao Pango."""
    pedacos = re.split(r"(`[^`]+`)", texto)
    saida = []
    for p in pedacos:
        if p.startswith("`") and p.endswith("`") and len(p) > 1:
            saida.append('<span font_family="Noto Sans Mono" size="92%">'
                         f"{html.escape(p[1:-1], quote=False)}</span>")
            continue
        p = html.escape(p, quote=False)
        p = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r"\1", p)
        p = re.sub(r"\*\*(.+?)\*\*", r"<b>\1</b>", p)
        p = re.sub(r"(?<![*\w])\*(?!\s)(.+?)(?<!\s)\*(?!\w)", r"<i>\1</i>", p)
        saida.append(p)
    return "".join(saida)


def ler_blocos(md):
    linhas = md.splitlines()
    blocos, i = [], 0
    while i < len(linhas):
        ln = linhas[i]
        if not ln.strip():
            i += 1
        elif ln.startswith("# "):
            blocos.append(("h1", inline(ln[2:].strip())))
            i += 1
        elif ln.startswith("## "):
            blocos.append(("h2", inline(ln[3:].strip())))
            i += 1
        elif m := re.match(r"!\[(.*)\]\((.+)\)\s*$", ln):
            nfig = sum(1 for b in blocos if b[0] == "fig") + 1
            blocos.append(("fig", nfig, m.group(1), m.group(2)))
            i += 1
        elif ln.startswith("|"):
            tab = []
            while i < len(linhas) and linhas[i].startswith("|"):
                cel = [c.strip() for c in linhas[i].strip().strip("|").split("|")]
                if not all(re.fullmatch(r":?-+:?", c) for c in cel):
                    tab.append([inline(c) for c in cel])
                i += 1
            blocos.append(("tabela", tab))
        elif ln.startswith("- "):
            itens = []
            while i < len(linhas) and linhas[i].startswith("- "):
                itens.append(inline(linhas[i][2:].strip()))
                i += 1
            blocos.append(("lista", itens))
        elif ln.startswith("> "):
            partes = []
            while i < len(linhas) and linhas[i].startswith(">"):
                partes.append(linhas[i][1:].strip())
                i += 1
            blocos.append(("nota", inline(" ".join(partes))))
        else:
            partes = []
            while i < len(linhas) and linhas[i].strip() and not re.match(r"(#|- |\||!\[|> )", linhas[i]):
                partes.append(linhas[i])
                i += 1
            txt = ""
            for p in partes:  # dois espacos no fim da linha = quebra de linha
                txt += inline(p.rstrip()) + ("\n" if p.endswith("  ") else " ")
            blocos.append(("p", txt.strip()))
    return blocos


# ------------------------------------------------------------ montagem PDF
A4 = (595.28, 841.89)
MARGEM_X, MARGEM_TOPO, MARGEM_BASE = 42, 34, 40
CORPO = "Noto Sans 9"
ESCALA_FIGURA = 0.64  # pontos do PDF por pixel do SVG


def hex_rgb(c):
    c = c.lstrip("#")
    return tuple(int(c[i:i + 2], 16) / 255 for i in (0, 2, 4))


class Montador:
    def __init__(self, cr, total_paginas, ao_virar):
        self.cr = cr
        self.ctx = PangoCairo.create_context(cr)
        PangoCairo.context_set_resolution(self.ctx, 72)
        opcoes = cairo.FontOptions()  # mesmas medidas no PDF e nas previas PNG
        opcoes.set_hint_metrics(cairo.HINT_METRICS_OFF)
        opcoes.set_hint_style(cairo.HINT_STYLE_NONE)
        PangoCairo.context_set_font_options(self.ctx, opcoes)
        self.total = total_paginas
        self.ao_virar = ao_virar
        self.pagina = 1
        self.larg = A4[0] - 2 * MARGEM_X
        self.y = MARGEM_TOPO
        self.pendentes = []  # figuras que nao couberam e vao para o topo da proxima pagina

    def layout(self, markup, largura, fonte=CORPO, alinhar=None):
        lay = Pango.Layout.new(self.ctx)
        lay.set_font_description(Pango.FontDescription.from_string(fonte))
        lay.set_width(int(largura * Pango.SCALE))
        lay.set_wrap(Pango.WrapMode.WORD_CHAR)
        if alinhar:
            lay.set_alignment(alinhar)
        lay.set_markup(markup, -1)
        return lay

    @staticmethod
    def altura(lay):
        return lay.get_extents()[1].height / Pango.SCALE

    def limite(self):
        return A4[1] - MARGEM_BASE

    def rodape(self):
        cr = self.cr
        cr.save()
        cr.set_source_rgb(*hex_rgb("#d1d5db"))
        cr.set_line_width(0.5)
        cr.move_to(MARGEM_X, A4[1] - 30)
        cr.line_to(A4[0] - MARGEM_X, A4[1] - 30)
        cr.stroke()
        lay = self.layout(f'<span foreground="{CINZA}">AES-128 em dois ESP32 via UART · '
                          f"Augusto e Gustavo · Página {self.pagina} de {self.total}</span>",
                          self.larg, "Noto Sans 7.5", Pango.Alignment.RIGHT)
        cr.move_to(MARGEM_X, A4[1] - 26)
        PangoCairo.show_layout(cr, lay)
        cr.restore()

    def nova_pagina(self):
        self.rodape()
        self.ao_virar()
        self.pagina += 1
        self.y = MARGEM_TOPO
        pendentes, self.pendentes = self.pendentes, []
        for fig in pendentes:
            self.figura_svg(*fig)

    def garantir(self, h):
        if self.y + h > self.limite():
            self.nova_pagina()

    def desenhar(self, lay, x):
        self.cr.move_to(x, self.y)
        PangoCairo.show_layout(self.cr, lay)

    def paragrafo(self, markup, x=None, largura=None, fonte=CORPO, depois=5, marcador=None):
        x = MARGEM_X if x is None else x
        largura = self.larg if largura is None else largura
        lay = self.layout(markup, largura, fonte)
        it = lay.get_iter()
        primeira = True
        topo_layout = None
        while True:
            _, log = it.get_line_extents()
            alt = log.height / Pango.SCALE
            if self.y + alt > self.limite():
                self.nova_pagina()
            if topo_layout is None:
                topo_layout = log.y / Pango.SCALE
            base = it.get_baseline() / Pango.SCALE - log.y / Pango.SCALE
            linha = it.get_line_readonly()
            if primeira and marcador:
                self.cr.set_source_rgb(*hex_rgb(AZUL))
                self.cr.arc(x - 8, self.y + base - 3, 1.6, 0, 6.3)
                self.cr.fill()
                primeira = False
            self.cr.set_source_rgb(*hex_rgb(TEXTO))
            self.cr.move_to(x + log.x / Pango.SCALE, self.y + base)
            PangoCairo.show_layout_line(self.cr, linha)
            self.y += alt
            if not it.next_line():
                break
        self.y += depois

    def titulo(self, markup):
        lay = self.layout(f'<span foreground="{AZUL}"><b>{markup}</b></span>', self.larg,
                          "Noto Sans 17")
        self.desenhar(lay, MARGEM_X)
        self.y += self.altura(lay) + 6

    def secao(self, markup):
        self.garantir(16 + 40)
        self.y += 6
        lay = self.layout(f'<span foreground="{AZUL}"><b>{markup}</b></span>', self.larg,
                          "Noto Sans 11.5")
        self.desenhar(lay, MARGEM_X)
        self.y += self.altura(lay) + 1
        self.cr.set_source_rgb(*hex_rgb("#bfdbfe"))
        self.cr.set_line_width(0.8)
        self.cr.move_to(MARGEM_X, self.y)
        self.cr.line_to(A4[0] - MARGEM_X, self.y)
        self.cr.stroke()
        self.y += 5

    def nota(self, markup):
        pad = 7
        lay = self.layout(markup, self.larg - 2 * pad - 3)
        h = self.altura(lay) + 2 * pad
        self.garantir(h)
        cr = self.cr
        cr.set_source_rgb(*hex_rgb(AZUL_CLARO))
        cr.rectangle(MARGEM_X, self.y, self.larg, h)
        cr.fill()
        cr.set_source_rgb(*hex_rgb(AZUL))
        cr.rectangle(MARGEM_X, self.y, 3, h)
        cr.fill()
        cr.set_source_rgb(*hex_rgb(TEXTO))
        self.cr.move_to(MARGEM_X + 3 + pad, self.y + pad)
        PangoCairo.show_layout(cr, lay)
        self.y += h + 7

    def lista(self, itens):
        for item in itens:
            self.paragrafo(item, x=MARGEM_X + 14, largura=self.larg - 14, depois=2.5,
                           marcador=True)
        self.y += 3

    def figura_svg(self, numero, legenda, caminho):
        h_svg = Rsvg.Handle.new_from_file(str(REL / caminho))
        ok, w, h = h_svg.get_intrinsic_size_in_pixels()
        largura = min(self.larg, w * ESCALA_FIGURA)
        altura = h * largura / w
        leg = self.layout(f'<span foreground="{CINZA}"><i><b>Figura {numero}.</b> '
                          f"{legenda}</i></span>", self.larg, "Noto Sans 8",
                          Pango.Alignment.CENTER)
        total = altura + 4 + self.altura(leg)
        if self.y + total > self.limite():
            self.pendentes.append((numero, legenda, caminho))
            return
        x = MARGEM_X + (self.larg - largura) / 2
        ret = Rsvg.Rectangle()
        ret.x, ret.y, ret.width, ret.height = x, self.y, largura, altura
        h_svg.render_document(self.cr, ret)
        self.y += altura + 3
        self.cr.set_source_rgb(*hex_rgb(TEXTO))
        self.desenhar(leg, MARGEM_X)
        self.y += self.altura(leg) + 7

    def tabela(self, linhas):
        fonte, pad = "Noto Sans 8", 3.2
        ncol = len(linhas[0])
        nat = [0.0] * ncol
        for row in linhas:
            for j, cel in enumerate(row):
                lay = self.layout(cel, 10000, fonte)
                nat[j] = max(nat[j], lay.get_extents()[1].width / Pango.SCALE + 2 * pad)
        largs = nat[:]
        if sum(largs) > self.larg:
            fixas = sorted(range(ncol), key=lambda j: nat[j])
            restante = self.larg
            for j in fixas[:-1]:
                largs[j] = min(nat[j], self.larg * 0.42)
                restante -= largs[j]
            largs[fixas[-1]] = restante
        else:
            largs[-1] += self.larg - sum(largs)
        alturas = []
        lays = []
        for i, row in enumerate(linhas):
            lr = [self.layout(f"<b>{c}</b>" if i == 0 else c, largs[j] - 2 * pad, fonte)
                  for j, c in enumerate(row)]
            lays.append(lr)
            alturas.append(max(self.altura(lay) for lay in lr) + 2 * pad)
        def linha(i, fundo):
            cr = self.cr  # muda a cada pagina nas previas PNG
            if fundo:
                cr.set_source_rgb(*hex_rgb(fundo))
                cr.rectangle(MARGEM_X, self.y, self.larg, alturas[i])
                cr.fill()
            x = MARGEM_X
            for j, lay in enumerate(lays[i]):
                cr.set_source_rgb(*hex_rgb(TEXTO))
                cr.move_to(x + pad, self.y + pad)
                PangoCairo.show_layout(cr, lay)
                x += largs[j]
            self.y += alturas[i]
            cr.set_source_rgb(*hex_rgb("#d1d5db"))
            cr.set_line_width(0.5)
            cr.move_to(MARGEM_X, self.y)
            cr.line_to(A4[0] - MARGEM_X, self.y)
            cr.stroke()

        self.garantir(alturas[0] + sum(alturas[1:3]))
        linha(0, "#dbeafe")
        for i in range(1, len(lays)):
            if self.y + alturas[i] > self.limite():
                self.nova_pagina()
                linha(0, "#dbeafe")  # repete o cabecalho
            linha(i, "#f9fafb" if i % 2 == 0 else None)
        self.y += 8

    def montar(self, blocos):
        for b in blocos:
            tipo = b[0]
            if tipo == "h1":
                self.titulo(b[1])
            elif tipo == "h2":
                self.secao(b[1])
            elif tipo == "p":
                self.paragrafo(b[1])
            elif tipo == "lista":
                self.lista(b[1])
            elif tipo == "nota":
                self.nota(b[1])
            elif tipo == "fig":
                if self.pendentes:  # mantem a ordem das figuras
                    self.pendentes.append(b[1:])
                else:
                    self.figura_svg(*b[1:])
            elif tipo == "tabela":
                self.tabela(b[1])
        while self.pendentes:
            self.nova_pagina()
        self.rodape()
        return self.pagina


def montar_pdf(blocos, destino, total):
    sup = cairo.PDFSurface(str(destino), *A4)
    sup.set_metadata(cairo.PDF_METADATA_TITLE, "AES-128 em dois ESP32 via UART")
    sup.set_metadata(cairo.PDF_METADATA_AUTHOR, "Augusto e Gustavo")
    cr = cairo.Context(sup)
    paginas = Montador(cr, total, cr.show_page).montar(blocos)
    sup.finish()
    return paginas


def montar_png(blocos, pasta, total, escala=1.6):
    pasta.mkdir(parents=True, exist_ok=True)
    estado = {"sup": None, "cr": None, "n": 0}

    def nova():
        sup = cairo.ImageSurface(cairo.FORMAT_RGB24, int(A4[0] * escala), int(A4[1] * escala))
        cr = cairo.Context(sup)
        cr.set_source_rgb(1, 1, 1)
        cr.paint()
        cr.scale(escala, escala)
        estado.update(sup=sup, cr=cr)
        return cr

    cr = nova()
    mont = Montador(cr, total, lambda: None)

    def virar():
        estado["n"] += 1
        estado["sup"].write_to_png(str(pasta / f"pagina-{estado['n']}.png"))
        mont.cr = nova()

    mont.ao_virar = virar
    mont.montar(blocos)
    estado["n"] += 1
    estado["sup"].write_to_png(str(pasta / f"pagina-{estado['n']}.png"))


def contar_paginas_pdf(caminho):
    """Conta as paginas lendo o PDF gerado (o cairo comprime a arvore de paginas)."""
    dados = caminho.read_bytes()
    texto = dados
    for m in re.finditer(rb"stream\r?\n(.*?)\r?\nendstream", dados, re.S):
        try:
            texto += zlib.decompress(m.group(1))
        except zlib.error:
            pass
    return len(re.findall(rb"/Type\s*/Page(?![s\w])", texto))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--png", type=Path, help="pasta para salvar previas PNG das paginas")
    args = ap.parse_args()
    figs = gerar_figuras()
    blocos = ler_blocos((REL / "relatorio.md").read_text(encoding="utf-8"))
    destino = REL / "relatorio.pdf"
    total = montar_pdf(blocos, destino, 0)       # 1a passada: conta as paginas
    total = montar_pdf(blocos, destino, total)   # 2a passada: rodape com o total
    if args.png:
        montar_png(blocos, args.png, total)
    print(f"figuras: {len(figs)} em {FIG.relative_to(RAIZ)}")
    no_arquivo = contar_paginas_pdf(destino)
    print(f"pdf: {destino.relative_to(RAIZ)} paginas={total} (no arquivo: {no_arquivo}) "
          f"limite={LIMITE_PAGINAS}")
    if max(total, no_arquivo) > LIMITE_PAGINAS:
        print("ERRO: o relatorio passou do limite de paginas do enunciado", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
