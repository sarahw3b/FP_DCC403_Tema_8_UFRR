/**
 * @file reporter.cpp
 * @brief Sintetizador de Relatórios
 *
 * O reporter é o ponto de saída da ferramenta para o usuário final.
 * Ele não realiza nenhum cálculo novo — apenas formata e apresenta
 * as informações já computadas pelos módulos anteriores.
 *
 * Estrutura do relatório quando SAT (race detectado):
 *   1. Cabeçalho: "DATA RACE DETECTADO!"
 *   2. Lista de todos os acessos extraídos (contexto completo)
 *   3. Pares conflitantes: mostra exatamente qual thread escreve e qual lê,
 *      em qual linha, e confirma ausência de mutex comum
 *   4. Sugestão de correção com exemplo de código pthread correto
 *
 * Estrutura do relatório quando UNSAT (código seguro):
 *   1. Mensagem de aprovação formal
 *   2. Confirmação de que todos os acessos estão sincronizados
 */

#include "reporter.h"
#include <iostream>

using namespace std;

void generateReport(const vector<Access>&   acessos,
                    const vector<Conflito>& conflitos,
                    bool hasRace) {

    cout << "\n\n- RELATÓRIO DE ANÁLISE: -\n";

    // Código aprovado 
    if (!hasRace) {
        cout << "Nenhum data race encontrado.\n";
        cout << "O código está seguro para execução concorrente.\n";
        return;
    }

    // Race detectado 
    cout << "DATA RACE DETECTADO!\n\n";

    // Seção 1: todos os acessos extraídos pelo parser
    // Permite ao usuário verificar se o parser identificou corretamente
    // as threads, variáveis e mutexes do código analisado
    cout << "ACESSOS EXTRAÍDOS:\n";
    for (const auto& a : acessos) {
        cout << "  Thread: "   << a.thread
             << ", Variavel: " << a.variavel
             << ", Tipo: "     << a.tipo
             << ", Linha: "    << a.linha
             << ", Mutex: "    << (a.mutex.empty() ? "nenhum" : a.mutex)
             << "\n";
    }

    // Seção 2: pares conflitantes — o contraexemplo estruturado
    // Para cada Conflito, mostra a trilha de execução que causa o race:
    // qual thread escreve, qual lê, em qual linha, e confirma ausência
    // de mutex comum protegendo ambos os acessos simultaneamente
    cout << "\nPARES CONFLITANTES:\n";
    for (const auto& c : conflitos) {
        cout << "  Escrita: " << c.escrita.thread
             << " (linha "   << c.escrita.linha << ")"
             << " em '"      << c.escrita.variavel << "'"
             << " | Mutex: " << (c.escrita.mutex.empty() ? "nenhum"
                                                         : c.escrita.mutex)
             << "\n";

        cout << "  Leitura: " << c.leitura.thread
             << " (linha "   << c.leitura.linha << ")"
             << " em '"      << c.leitura.variavel << "'"
             << " | Mutex: " << (c.leitura.mutex.empty() ? "nenhum"
                                                         : c.leitura.mutex)
             << "\n";

        cout << "  → Nenhum mutex comum protege ambos os acessos.\n\n";
    }

    // Seção 3: sugestão de correção
    // Orienta o desenvolvedor sobre como corrigir o race detectado
    cout << "SUGESTÃO DE CORREÇÃO:\n";
    cout << "  Utilize o mesmo mutex para proteger ambos os acessos"
            " à variável compartilhada.\n";
    cout << "  Exemplo:\n";
    cout << "    pthread_mutex_lock(&mutex);\n";
    cout << "    // acesso à variável\n";
    cout << "    pthread_mutex_unlock(&mutex);\n";
}
