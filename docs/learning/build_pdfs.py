#!/usr/bin/env python3
"""Convert the DMS learning-pack Markdown guides into styled A4 PDFs.

Usage:
    python3 build_pdfs.py            # convert every 0*.md in this folder
    python3 build_pdfs.py 01_*.md    # convert specific files

Requires: markdown, pymdown-extensions, pygments, weasyprint (all pip-installable).
Output PDFs are written to ./pdf/<same-stem>.pdf
"""
import sys
import re
import pathlib
import datetime

import markdown
from markdown.extensions.toc import TocExtension
from pygments.formatters import HtmlFormatter
from weasyprint import HTML

HERE = pathlib.Path(__file__).resolve().parent
OUT = HERE / "pdf"
OUT.mkdir(exist_ok=True)

# Pygments code-highlight CSS (a calm, print-friendly light theme).
PYGMENTS_CSS = HtmlFormatter(style="friendly").get_style_defs(".codehilite")

BASE_CSS = """
@page {
    size: A4;
    margin: 20mm 18mm 22mm 18mm;
    @bottom-center {
        content: "Driver Monitoring System — Study Pack";
        font-size: 8pt; color: #94a3b8;
    }
    @bottom-right {
        content: "Page " counter(page) " of " counter(pages);
        font-size: 8pt; color: #94a3b8;
    }
}
@page :first { @bottom-center { content: ""; } @bottom-right { content: ""; } }

html { font-size: 10.5pt; }
body {
    font-family: "DejaVu Sans", "Noto Sans", Arial, sans-serif;
    color: #1f2933; line-height: 1.5;
}

/* Cover block (first heading + subtitle) */
h1 {
    font-size: 22pt; color: #0f172a; line-height: 1.2;
    border-bottom: 3px solid #2563eb; padding-bottom: 8px; margin-top: 0;
    page-break-before: avoid;
}
h2 {
    font-size: 15.5pt; color: #1d4ed8; margin-top: 1.6em;
    border-bottom: 1px solid #cbd5e1; padding-bottom: 3px;
    page-break-after: avoid;
}
h3 { font-size: 12.5pt; color: #0f172a; margin-top: 1.2em; page-break-after: avoid; }
h4 { font-size: 11pt; color: #334155; margin-top: 1em; page-break-after: avoid; }
p, li { orphans: 2; widows: 2; }

a { color: #2563eb; text-decoration: none; }

code {
    font-family: "DejaVu Sans Mono", "Noto Sans Mono", monospace;
    font-size: 9pt; background: #f1f5f9; color: #b91c1c;
    padding: 1px 4px; border-radius: 3px;
}
pre {
    background: #f8fafc; border: 1px solid #e2e8f0; border-left: 4px solid #2563eb;
    border-radius: 5px; padding: 10px 12px; overflow-x: auto;
    page-break-inside: avoid; font-size: 8.8pt; line-height: 1.4;
}
pre code { background: none; color: #0f172a; padding: 0; font-size: 8.8pt; }

table {
    border-collapse: collapse; width: 100%; margin: 12px 0;
    font-size: 9pt; page-break-inside: avoid;
}
th, td { border: 1px solid #cbd5e1; padding: 5px 8px; text-align: left; vertical-align: top; }
th { background: #eff6ff; color: #1e3a8a; font-weight: 600; }
tr:nth-child(even) td { background: #f8fafc; }

blockquote {
    margin: 12px 0; padding: 8px 14px; background: #fffbeb;
    border-left: 4px solid #f59e0b; color: #78350f; border-radius: 4px;
}
blockquote p { margin: 4px 0; }

ul, ol { margin: 8px 0; padding-left: 22px; }
li { margin: 3px 0; }

hr { border: none; border-top: 1px solid #cbd5e1; margin: 20px 0; }

/* Table of contents produced by [TOC] */
.toc { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px;
       padding: 10px 18px; page-break-inside: avoid; }
.toc ul { list-style: none; padding-left: 14px; }
.toc > ul { padding-left: 0; }
.toc a { color: #334155; }

img { max-width: 100%; }
"""


def build_one(md_path: pathlib.Path) -> pathlib.Path:
    text = md_path.read_text(encoding="utf-8")
    md = markdown.Markdown(
        extensions=[
            "extra",            # tables, fenced_code, footnotes, etc.
            "codehilite",       # pygments highlighting
            "sane_lists",
            "admonition",
            TocExtension(permalink=False, toc_depth="2-3"),
        ],
        extension_configs={
            "codehilite": {"guess_lang": False, "noclasses": False},
        },
    )
    # If the document doesn't already ask for a TOC, insert one after the first H1.
    if "[TOC]" not in text and "[toc]" not in text:
        lines = text.split("\n")
        for i, ln in enumerate(lines):
            if ln.startswith("# "):
                lines.insert(i + 1, "\n[TOC]\n")
                break
        text = "\n".join(lines)

    body = md.convert(text)
    stamp = datetime.date.today().isoformat()
    html = f"""<!doctype html><html><head><meta charset="utf-8">
<style>{PYGMENTS_CSS}\n{BASE_CSS}
.docmeta {{ color:#64748b; font-size:9pt; margin: 4px 0 18px 0; }}
</style></head><body>
{body}
</body></html>"""

    out = OUT / (md_path.stem + ".pdf")
    HTML(string=html, base_url=str(md_path.parent)).write_pdf(str(out))
    return out


def main():
    args = sys.argv[1:]
    if args:
        files = []
        for a in args:
            files.extend(sorted(HERE.glob(a)))
    else:
        files = sorted(p for p in HERE.glob("0*.md"))
    if not files:
        print("No Markdown files found to convert.")
        return
    for md_path in files:
        out = build_one(md_path)
        size_kb = out.stat().st_size / 1024
        print(f"  {md_path.name}  ->  {out.relative_to(HERE)}  ({size_kb:.0f} KB)")


if __name__ == "__main__":
    main()
