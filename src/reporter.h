/**
 * @file reporter.h
 * @brief Sintetizador de Relatórios de Auditoria
 *
 * Este módulo recebe os resultados dos três módulos anteriores e produz
 * um relatório de auditoria estruturado no terminal, legível por humanos.
 *
 * Quando o solver retorna SAT (race detectado), o relatório exibe:
 *   - Todos os acessos extraídos pelo parser (thread, variável, tipo, linha, mutex)
 *   - Os pares conflitantes específicos: qual thread escreve e qual lê,
 *     em qual linha, e confirma a ausência de mutex comum
 *   - Uma sugestão de correção com exemplo de código
 *
 * Quando o solver retorna UNSAT (código seguro), o relatório confirma
 * que nenhum data race foi detectado e que o código está seguro para
 * execução concorrente dentro do escopo analisado.
 *
 * Disciplina: DCC403 — Sistemas Operacionais — UFRR — 2026
 * Tema 8: Verificador Formal de Corridas de Dados para Código Concorrente em C
 */

#ifndef REPORTER_H
#define REPORTER_H

#include "parser.h"
#include "cnf_gen.h"
#include <vector>
#include <string>

using namespace std;

/**
 * @brief Gera e imprime o relatório de auditoria no terminal.
 *
 * @param acessos   Vetor de acessos extraídos pelo Módulo 1 (parser)
 * @param conflitos Vetor de pares conflitantes gerados pelo Módulo 2 (cnf_gen)
 * @param hasRace   Veredito do Módulo 3: true = SAT (race), false = UNSAT (seguro)
 */
void generateReport(const vector<Access>&   acessos,
                    const vector<Conflito>& conflitos,
                    bool hasRace);

#endif // REPORTER_H
