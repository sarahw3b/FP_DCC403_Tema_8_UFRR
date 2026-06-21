/**
 * @file cnf_gen.h
 * @brief Gerador de Fórmulas Lógicas em CNF
 *
 * Este módulo recebe o vetor de acessos produzido pelo Módulo 1 (parser)
 * e o transforma em uma fórmula booleana no formato CNF (Forma Normal
 * Conjuntiva), gravada em um arquivo DIMACS compatível com o MiniSAT.
 *
 * Fundamentação matemática:
 *   Para cada par de acessos (W_i, R_j) de threads distintas sobre a mesma
 *   variável global x, onde W_i é uma escrita, define-se:
 *
 *     Race(W_i, R_j) ⟺ W_i ∧ R_j ∧ ¬P(W_i, R_j)
 *
 *   onde P(W_i, R_j) = true se e somente se ambos os acessos estão
 *   protegidos pelo mesmo mutex (mesmo nome, não vazio).
 *
 *   Associando índices proposicionais:
 *     idW ≡ "o acesso de escrita W_i ocorre"
 *     idR ≡ "o acesso de leitura R_j ocorre"
 *     idP ≡ "há proteção mutex comum"
 *
 *   As cláusulas DIMACS geradas por par inseguro são:
 *     idW  0    (W_i é verdadeiro)
 *     idR  0    (R_j é verdadeiro)
 *    -idP  0    (P é falso — sem proteção)
 *
 *   Se o solver retorna SAT para essa fórmula, existe uma valoração onde
 *   todos os três literais são satisfeitos simultaneamente → race detectado.
 *
 * Disciplina: DCC403 — Sistemas Operacionais — UFRR — 2026
 * Tema 8: Verificador Formal de Corridas de Dados para Código Concorrente em C
 */

#ifndef CNF_GEN_H
#define CNF_GEN_H

#include "parser.h"
#include <vector>
#include <string>

using namespace std;

/**
 * @struct Conflito
 * @brief Representa um par de acessos conflitantes sem proteção mutex comum.
 *
 * Cada Conflito corresponde a um potencial data race: uma escrita por uma
 * thread e uma leitura (ou escrita) por outra thread na mesma variável,
 * sem que ambas estejam protegidas pelo mesmo mutex.
 *
 * Os campos idW, idR e idP são os índices das variáveis proposicionais
 * atribuídas a este par no arquivo DIMACS. Eles são usados pelo Módulo 4
 * (reporter) para reconstruir o contraexemplo a partir da valoração do solver.
 */
struct Conflito {
    Access escrita;  ///< Acesso de escrita (tipo == "write")
    Access leitura;  ///< Acesso de leitura (tipo == "read") ou segunda escrita
    int    idW;      ///< Índice proposicional do acesso de escrita no DIMACS
    int    idR;      ///< Índice proposicional do acesso de leitura no DIMACS
    int    idP;      ///< Índice proposicional da proteção mutex no DIMACS
};

/**
 * @brief Gera o arquivo CNF no formato DIMACS a partir dos acessos extraídos.
 *
 * Algoritmo:
 *   1. Agrupa os acessos por variável global.
 *   2. Para cada variável, forma todos os pares (escrita de T_i, leitura de T_j)
 *      com i ≠ j.
 *   3. Verifica se o par possui mutex comum (mesmo nome não vazio em ambos).
 *   4. Pares sem mutex comum geram um Conflito e três cláusulas DIMACS.
 *   5. Se não há nenhum par inseguro, grava "p cnf 0 0" (fórmula vazia,
 *      trivialmente UNSAT do ponto de vista da verificação de race).
 *
 * @param acessos  Vetor de acessos produzido por parseFile()
 * @param filename Caminho do arquivo .cnf a ser gerado
 * @return Vetor de Conflito com os pares inseguros encontrados (vazio se seguro)
 */
vector<Conflito> generateCNF(const vector<Access>& acessos,
                              const string& filename);

#endif // CNF_GEN_H
