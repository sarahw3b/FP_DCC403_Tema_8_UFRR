# FormalRace-Checker

Verificador formal de corridas de dados para código C concorrente (pthreads).

## Disciplina
DCC403 — Sistemas Operacionais — UFRR — 2026

## Descrição
Ferramenta de análise estática que detecta data races em programas C com pthreads
usando SAT Solving (MiniSAT). Traduz propriedades de concorrência em fórmulas CNF
e prova matematicamente a presença ou ausência de corridas de dados.

## Dependências
- g++ 17 ou superior
- MiniSAT (`sudo apt install minisat`)

## Como compilar
```bash
make
```

## Como usar
```bash
./formalrace-checker tests/cenario_a.c
```

## Estrutura do projeto
- `src/` — código-fonte dos 4 módulos
- `tests/` — cenários de teste obrigatórios
- `docs/` — artigo técnico

## Autor
Sarah Caroline Amaral Pereira — Universidade Federal de Roraima
