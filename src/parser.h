/**
 * @file parser.h
 * @brief Parser Estático do FormalRace-Checker
 *
 * Este módulo realiza a análise léxica e sintática superficial de arquivos
 * de código-fonte C concorrente. Sem compilar ou executar o programa, ele
 * varre o texto linha por linha e extrai três categorias de informação:
 *
 *   1. Variáveis globais — declaradas fora de qualquer função, são as
 *      candidatas a sofrer corridas de dados entre threads.
 *
 *   2. Funções de thread — identificadas pelo terceiro argumento de
 *      pthread_create(), são os pontos de entrada de cada thread.
 *
 *   3. Acessos a variáveis globais — para cada função de thread, registra
 *      cada leitura ou escrita a uma variável global, a linha onde ocorre
 *      e qual mutex (se algum) estava ativo naquele ponto.
 *
 * A saída é um vetor de structs Access que alimenta o Módulo 2 (cnf_gen).
 *
 * Limitações conhecidas (documentadas no artigo):
 *   - Não rastreia acessos via ponteiros (ex: *p = 42 onde p = &global)
 *   - Não realiza análise interprocedural (locks em funções auxiliares
 *     chamadas pela thread não são associados ao escopo da thread)
 *
 * Disciplina: DCC403 — Sistemas Operacionais — UFRR — 2026
 * Tema 8: Verificador Formal de Corridas de Dados para Código Concorrente em C
 */

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

using namespace std;

/**
 * @struct Access
 * @brief Representa um único acesso (leitura ou escrita) a uma variável global
 *        realizado por uma thread específica em uma linha específica do código.
 *
 * Cada instância de Access corresponde a um evento do conjunto
 * E = {read(x), write(x)} da semântica de entrelaçamento formal.
 * O campo mutex registra qual mutex estava na pilha de locks ativos no
 * momento do acesso — vazio ("") indica que o acesso não está protegido.
 */
struct Access {
    string thread;    ///< Nome da função que executa como thread (ex: "writer")
    string variavel;  ///< Nome da variável global acessada (ex: "contador")
    string tipo;      ///< Tipo do acesso: "read" (leitura) ou "write" (escrita)
    int    linha;     ///< Número da linha no arquivo .c onde o acesso ocorre
    string mutex;     ///< Nome do mutex ativo no momento do acesso; "" se nenhum
};

/**
 * @brief Realiza o parsing estático de um arquivo C concorrente.
 *
 * A função executa três passagens sobre o arquivo:
 *
 * Passagem 1 — Identificação de variáveis globais:
 *   Varre o arquivo contando o balanço de chaves '{' e '}'. Declarações
 *   de variáveis encontradas com balanço == 0 (fora de qualquer função)
 *   são registradas como variáveis globais candidatas a race condition.
 *   Tipos pthread_mutex_t são explicitamente ignorados.
 *
 * Passagem 2 — Identificação de funções de thread:
 *   Busca chamadas a pthread_create() e extrai o terceiro argumento
 *   (ponteiro para a função de thread) via expressão regular.
 *
 * Passagem 3 — Análise de acessos:
 *   Para cada função de thread identificada, varre suas linhas internas.
 *   Mantém uma pilha (mutexStack) de mutexes ativos: empilha no
 *   pthread_mutex_lock() e desempilha no pthread_mutex_unlock().
 *   Detecta escritas (operadores =, +=, -=, ++, --) e leituras de
 *   variáveis globais, registrando o mutex do topo da pilha.
 *
 * @param filename Caminho para o arquivo .c a ser analisado
 * @return Vetor de Access com todos os acessos encontrados
 */
vector<Access> parseFile(const string& filename);

#endif // PARSER_H
