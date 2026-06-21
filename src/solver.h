/**
 * @file solver.h
 * @brief Conector com o SAT Solver (MiniSAT)
 *
 * Este módulo encapsula a integração com o MiniSAT, um solver SAT industrial
 * de código aberto baseado no algoritmo DPLL com aprendizado de cláusulas
 * (CDCL — Conflict-Driven Clause Learning).
 *
 * Responsabilidades:
 *   1. Verificar o arquivo CNF gerado pelo Módulo 2 para decidir se
 *      há cláusulas a satisfazer (evita invocar o solver desnecessariamente).
 *   2. Invocar o MiniSAT como processo externo via system(), passando o
 *      arquivo DIMACS como entrada e um arquivo de resultado como saída.
 *   3. Ler o arquivo de resultado e retornar true (SAT = race detectado)
 *      ou false (UNSAT = código seguro).
 *
 * Por que MiniSAT e não Z3?
 *   O MiniSAT é mais simples de instalar (apt install minisat) e sua
 *   interface via arquivo DIMACS é direta. O Z3 oferece suporte a SMT
 *   (Satisfatibilidade Módulo Teorias), útil para modelos mais expressivos,
 *   mas sua integração via API ou linha de comando é mais complexa.
 *   Para os cenários deste projeto, SAT puro é suficiente.
 *
 * Disciplina: DCC403 — Sistemas Operacionais — UFRR — 2026
 * Tema 8: Verificador Formal de Corridas de Dados para Código Concorrente em C
 */

#ifndef SOLVER_H
#define SOLVER_H

#include <string>

using namespace std;

/**
 * @brief Executa o MiniSAT sobre o arquivo CNF e retorna o veredito.
 *
 * Fluxo interno:
 *   1. Lê o cabeçalho "p cnf N M" do arquivo DIMACS.
 *   2. Se M == 0 (sem cláusulas), retorna false sem chamar o solver —
 *      fórmula vazia significa ausência de pares conflitantes.
 *   3. Caso contrário, invoca "minisat <cnfFile> <resultFile>" via system().
 *   4. Lê a primeira linha relevante do arquivo de resultado:
 *        "SAT"  ou "SATISFIABLE"   → retorna true  (race detectado)
 *        "UNSAT" ou "UNSATISFIABLE" → retorna false (código seguro)
 *
 * @param cnfFile    Caminho do arquivo .cnf gerado pelo Módulo 2
 * @param resultFile Caminho do arquivo de saída onde o MiniSAT escreve o veredito
 * @return true se SAT (data race detectado), false se UNSAT (código seguro)
 */
bool runSolver(const string& cnfFile, const string& resultFile);

#endif // SOLVER_H