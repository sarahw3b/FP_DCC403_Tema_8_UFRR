/**
 * @file main.cpp
 * @brief FormalRace-Checker — Verificador Formal de Corridas de Dados
 *
 * Ponto de entrada da ferramenta. Orquestra a execução sequencial dos
 * quatro módulos do pipeline de verificação:
 *
 *   [Código C] → Parser/AST → Gerador CNF → SAT Solver → Relatório
 *
 * Uso:
 *   ./formalrace-checker <arquivo.c>
 *
 * Exemplos:
 *   ./formalrace-checker tests/cenario_a.c   # espera: RACE DETECTADO
 *   ./formalrace-checker tests/cenario_b.c   # espera: CÓDIGO SEGURO
 *   ./formalrace-checker tests/cenario_c.c   # espera: CÓDIGO SEGURO
 *
 * Arquivos gerados durante a execução:
 *   output.cnf    — fórmula CNF no formato DIMACS (Módulo 2)
 *   resultado.txt — veredito e valoração do MiniSAT (Módulo 3)
 *
 * Dependências:
 *   - g++ com suporte a C++11 ou superior
 *   - MiniSAT 2.x instalado e disponível no PATH (apt install minisat)
 *
 * Compilação:
 *   make
 *
 * Disciplina: DCC403 — Sistemas Operacionais — UFRR — 2026
 * Tema 8: Verificador Formal de Corridas de Dados para Código Concorrente em C
 * Repositório: https://github.com/sarahw3b/FP_DCC403_Tema_8_UFRR
 */

#include <iostream>
#include <vector>
#include "src/parser.h"
#include "src/cnf_gen.h"
#include "src/solver.h"
#include "src/reporter.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " arquivo.c" << endl;
        return 1;
    }

    // ─ Parser ─
    // Lê o arquivo .c e extrai todos os acessos a variáveis globais
    // realizados por funções de thread, com informação de mutex ativo.
    vector<Access> acessos = parseFile(argv[1]);

    // Exibe os acessos extraídos para transparência do processo
    cout << "\nACESSOS EXTRAÍDOS:\n\n";
    for (const auto& a : acessos) {
        cout << "Thread: "   << a.thread
             << ", Variavel: " << a.variavel
             << ", Tipo: "     << a.tipo
             << ", Linha: "    << a.linha
             << ", Mutex: "    << (a.mutex.empty() ? "nenhum" : a.mutex)
             << endl;
    }

    // ─ Gerador de CNF ─
    // Identifica pares conflitantes e gera o arquivo DIMACS para o solver.
    vector<Conflito> conflitos = generateCNF(acessos, "output.cnf");

    // ─ Solver Connector ─
    // Invoca o MiniSAT sobre o CNF gerado e retorna o veredito booleano.
    bool hasRace = runSolver("output.cnf", "resultado.txt");

    // Imprime o veredito resumido antes do relatório detalhado
    cout << "\n\nRESULTADO:\n";
    if (hasRace)
        cout << "DATA RACE DETECTADO!" << endl;
    else
        cout << "Nenhum data race encontrado." << endl;

    // ─ Relatório ─
    // Gera o relatório de auditoria completo com contraexemplos (se houver).
    generateReport(acessos, conflitos, hasRace);

    return 0;
}
