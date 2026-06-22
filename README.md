# FormalRace-Checker

> Verificador Formal de Corridas de Dados para Código Concorrente em C

**Disciplina:** DCC403 — Sistemas Operacionais  
**Instituição:** Universidade Federal de Roraima (UFRR)  
**Tema:** 8 — Desenvolvimento de um Verificador Formal de Corridas de Dados para Código Concorrente em C  
**Autora:** Sarah C. A. Pereira  
**Ambiente de desenvolvimento:** Linux Fedora — KDE Plasma  

---

## O que é este projeto

O **FormalRace-Checker** é uma ferramenta de análise estática formal que detecta
*data races* (corridas de dados) em programas C concorrentes baseados em
POSIX Threads (pthreads), **sem executar o programa analisado**.

Em vez de testar o programa dinamicamente, a ferramenta:

1. Lê o código-fonte C e extrai todos os acessos a variáveis globais realizados por threads
2. Modela as propriedades de concorrência como fórmulas booleanas em Forma Normal Conjuntiva (CNF)
3. Submete as fórmulas ao SAT Solver **MiniSAT**
4. Gera um relatório de auditoria indicando exatamente qual thread, em qual linha e sobre qual variável ocorre a corrida de dados

A detecção é baseada na definição formal:

```
Race(W, R) ⟺ W ∧ R ∧ ¬P(W, R)
```

Onde `W` é uma escrita, `R` é uma leitura de thread diferente na mesma variável,
e `P` indica se ambos os acessos estão protegidos pelo mesmo mutex.
Se o solver responde **SAT** → race detectado. Se responde **UNSAT** → código seguro.

---

## Estrutura do Repositório

```
FP_DCC403_Tema_8_UFRR/
├── src/
│   ├── parser.h          # Módulo 1: interface do parser estático
│   ├── parser.cpp        # Módulo 1: extrai threads, globais, acessos e mutexes
│   ├── cnf_gen.h         # Módulo 2: interface do gerador de fórmulas CNF
│   ├── cnf_gen.cpp       # Módulo 2: gera arquivo DIMACS para o MiniSAT
│   ├── solver.h          # Módulo 3: interface do conector com o MiniSAT
│   ├── solver.cpp        # Módulo 3: invoca o MiniSAT e lê o veredito
│   ├── reporter.h        # Módulo 4: interface do sintetizador de relatórios
│   └── reporter.cpp      # Módulo 4: gera relatório de auditoria legível
├── tests/
│   ├── cenario_a.c       # Cenário A: escrita sem mutex → esperado SAT (race)
│   ├── cenario_b.c       # Cenário B: mutex correto → esperado UNSAT (seguro)
│   └── cenario_c.c       # Cenário C: locks aninhados assimétricos → UNSAT
├── docs/
│   └── artigo_formalrace.pdf   # Artigo técnico no padrão SBC
├── main.cpp              # Ponto de entrada — orquestra os 4 módulos
├── Makefile              # Automação de compilação
└── README.md             # Este arquivo
```

### Descrição dos Módulos

| Módulo | Arquivo | Responsabilidade |
|--------|---------|-----------------|
| 1 — Parser | `src/parser.cpp` | Lê o `.c` linha a linha, identifica variáveis globais, funções de thread e registra cada acesso com informação de mutex ativo |
| 2 — Gerador CNF | `src/cnf_gen.cpp` | Para cada par de acessos conflitantes, gera cláusulas no formato DIMACS e salva em `output.cnf` |
| 3 — Solver Connector | `src/solver.cpp` | Invoca o MiniSAT via `system()`, lê o arquivo de resultado e retorna SAT ou UNSAT |
| 4 — Relatório | `src/reporter.cpp` | Formata e imprime o relatório de auditoria com contraexemplos estruturados |

---

## Dependências

| Ferramenta | Versão mínima | Finalidade |
|-----------|---------------|-----------|
| g++ | 5.0+ | Compilador C++ |
| make | qualquer | Automação de build |
| MiniSAT | 2.x | SAT Solver para verificação formal |

---

## Instalação das Dependências

### Linux Fedora (ambiente de desenvolvimento)

```bash
# Instalar o compilador g++ e o make
sudo dnf install gcc-c++ make

# Instalar o MiniSAT
sudo dnf install minisat

# Verificar instalações
g++ --version
minisat --help
```

### Linux Ubuntu / Debian

```bash
sudo apt update
sudo apt install g++ make minisat
```

### Verificar se o MiniSAT está funcionando

Crie um arquivo de teste e rode o solver manualmente:

```bash
cat > /tmp/teste.cnf << 'EOF'
p cnf 3 3
1 0
2 0
-3 0
EOF

minisat /tmp/teste.cnf /tmp/resultado.txt
cat /tmp/resultado.txt
```

Saída esperada:
```
SAT
1 2 -3 0
```

---

## Como Compilar

Dentro da pasta do projeto:

```bash
make
```

Para limpar os arquivos compilados:

```bash
make clean
```

O executável gerado é `./formalrace-checker`.

---

## Como Usar

```bash
./formalrace-checker <arquivo.c>
```

O arquivo analisado deve ser um programa C com `#include <pthread.h>` usando
`pthread_create`, variáveis globais e opcionalmente `pthread_mutex_lock/unlock`.

---

## Exemplos de Execução

### Cenário A — Acesso sem proteção mutex

Código analisado (`tests/cenario_a.c`): duas threads acessando a variável global
`contador`, uma sem mutex.

```bash
./formalrace-checker tests/cenario_a.c
```

Saída:

```
ACESSOS EXTRAÍDOS:
Thread: writer, Variavel: contador, Tipo: write, Linha: 7, Mutex: nenhum
Thread: reader, Variavel: contador, Tipo: read, Linha: 12, Mutex: nenhum

Par conflitante: writer escreve, reader lê em contador (sem mutex comum)
CNF gerado em output.cnf (3 variáveis, 3 cláusulas)

RESULTADO: DATA RACE DETECTADO!

- RELATÓRIO DE ANÁLISE: -
DATA RACE DETECTADO!

ACESSOS EXTRAÍDOS:
  Thread: writer, Variavel: contador, Tipo: write, Linha: 7, Mutex: nenhum
  Thread: reader, Variavel: contador, Tipo: read, Linha: 12, Mutex: nenhum

PARES CONFLITANTES:
  Escrita: writer (linha 7) em 'contador' | Mutex: nenhum
  Leitura: reader (linha 12) em 'contador' | Mutex: nenhum
  → Nenhum mutex comum protege ambos os acessos.

SUGESTÃO DE CORREÇÃO:
  Utilize o mesmo mutex para proteger ambos os acessos à variável compartilhada.
  Exemplo:
    pthread_mutex_lock(&mutex);
    // acesso à variável
    pthread_mutex_unlock(&mutex);
```

---

### Cenário B — Sincronização correta

Código analisado (`tests/cenario_b.c`): todos os acessos protegidos pelo mesmo mutex.

```bash
./formalrace-checker tests/cenario_b.c
```

Saída:

```
ACESSOS EXTRAÍDOS:
Thread: writer, Variavel: contador, Tipo: write, Linha: 8, Mutex: mutex
Thread: reader, Variavel: contador, Tipo: read, Linha: 15, Mutex: mutex

Nenhum par conflitante encontrado. Sem data races.

RESULTADO: Nenhum data race encontrado.

- RELATÓRIO DE ANÁLISE: -
Nenhum data race encontrado.
O código está seguro para execução concorrente.
```

---

### Cenário C — Locks aninhados assimétricos

Código analisado (`tests/cenario_c.c`): duas variáveis globais (`saldo` e `estoque`),
dois mutexes (`mutex1` e `mutex2`), adquiridos em ordem inversa pelas duas threads.

```bash
./formalrace-checker tests/cenario_c.c
```

Saída:

```
ACESSOS EXTRAÍDOS:
Thread: thread1, Variavel: saldo,    Tipo: write, Linha: 12, Mutex: mutex1
Thread: thread1, Variavel: estoque,  Tipo: write, Linha: 14, Mutex: mutex2
Thread: thread2, Variavel: estoque,  Tipo: write, Linha: 23, Mutex: mutex2
Thread: thread2, Variavel: saldo,    Tipo: write, Linha: 25, Mutex: mutex1

Nenhum par conflitante encontrado. Sem data races.

RESULTADO: Nenhum data race encontrado.

- RELATÓRIO DE ANÁLISE: -
Nenhum data race encontrado.
O código está seguro para execução concorrente.
```

> **Nota:** embora a ordem de aquisição dos locks no Cenário C configure
> risco de *deadlock* (espera circular), não há *data race* — cada variável
> é sempre acessada sob o mesmo mutex nas duas threads. O FormalRace-Checker
> verifica exclusivamente corridas de dados, não deadlocks.

---

## Resumo dos Resultados dos Testes

| Cenário | Descrição | Resultado Esperado | Resultado Obtido |
|---------|-----------|-------------------|-----------------|
| A | Escrita sem mutex | SAT — RACE DETECTADO | ✅ SAT |
| B | Mutex correto | UNSAT — SEGURO | ✅ UNSAT |
| C | Locks aninhados assimétricos | UNSAT — SEGURO | ✅ UNSAT |

**Precisão: 100%** nos três cenários obrigatórios.

---

## Limitações Conhecidas

- **Acesso via ponteiros:** escritas do tipo `*p = 42` onde `p = &global` não são detectadas
- **Análise interprocedural:** locks adquiridos dentro de funções auxiliares chamadas pela thread não são rastreados
- **Escopo:** projetado para programas de pequeno a médio porte com padrões explícitos de sincronização

Essas limitações estão documentadas na Seção 6 do artigo técnico.

---

## Artigo Técnico

O artigo completo no padrão SBC está disponível em `docs/artigo_formalrace.pdf`.

---

## Referências

- CLARKE et al. *Model Checking*. MIT Press, 2ª ed., 2018.
- BAIER; KATOEN. *Principles of Model Checking*. MIT Press, 2008.
- EÉN; SÖRENSSON. *An Extensible SAT-Solver*. SAT 2003.
- KROENING; STRICHMAN. *Decision Procedures*. Springer, 2016.
- ENGLER; ASHCRAFT. *RacerX*. SOSP 2003.
- FLANAGAN; FREUND. *FastTrack*. PLDI 2009.

---

## Declaração de Uso de IA

Durante o desenvolvimento deste projeto foram utilizadas as ferramentas
**Claude** (Anthropic) e **DeepSeek** (DeepSeek AI) como apoio técnico na estruturação do pipeline, revisão de código e redação.
Todo o conteúdo conceitual, código-fonte e cenários de teste foram
supervisionados e são de responsabilidade exclusiva da autora.
