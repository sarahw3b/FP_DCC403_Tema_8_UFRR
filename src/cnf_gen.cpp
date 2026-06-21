/**
 * @file cnf_gen.cpp
 * @brief Implementação do Gerador de Fórmulas CNF
 *
 * Este módulo implementa a tradução das propriedades de concorrência
 * detectadas pelo parser em fórmulas booleanas no formato DIMACS CNF.
 *
 * Formato DIMACS gerado:
 *   Linha de cabeçalho: "p cnf <num_variaveis> <num_clausulas>"
 *   Cada cláusula é uma linha de literais inteiros terminada por 0.
 *   Literais positivos = variável afirmada; negativos = variável negada.
 *
 *   Exemplo para um par conflitante com idW=1, idR=2, idP=3:
 *     p cnf 3 3
 *     1 0       ← a escrita ocorre (W_1 = true)
 *     2 0       ← a leitura ocorre (R_2 = true)
 *     -3 0      ← sem proteção mutex (P = false)
 */

#include "cnf_gen.h"
#include <iostream>
#include <map>
#include <fstream>

using namespace std;

vector<Conflito> generateCNF(const vector<Access>& acessos,
                              const string& filename) {

    // Passo 1: Agrupar acessos por variável global 
    // Usar um map garante que analisamos cada variável independentemente,
    // evitando cruzar acessos de variáveis diferentes.
    map<string, vector<Access>> acessosPorVariavel;
    for (const auto& a : acessos)
        acessosPorVariavel[a.variavel].push_back(a);

    vector<Conflito> conflitos;

    // Passo 2: Identificar pares conflitantes 
    for (const auto& par : acessosPorVariavel) {
        const string&         variavel = par.first;
        const vector<Access>& accs     = par.second;

        // Separa escritas de leituras para formar pares (W_i, R_j)
        vector<Access> writes, reads;
        for (const auto& a : accs) {
            if (a.tipo == "write") writes.push_back(a);
            else                   reads.push_back(a);
        }

        // Para cada par (escrita, leitura) de threads distintas:
        for (const auto& w : writes) {
            for (const auto& r : reads) {
                // Acesso da mesma thread não constitui race — pula
                if (w.thread == r.thread) continue;

                // Verifica proteção: ambos precisam ter o MESMO mutex
                // não-vazio. Se um está protegido e o outro não (ou por
                // mutex diferente), o par é inseguro.
                bool temMutexComum = (!w.mutex.empty() &&
                                      w.mutex == r.mutex);

                if (!temMutexComum) {
                    // Registra o par como conflitante
                    Conflito c;
                    c.escrita = w;
                    c.leitura = r;
                    // idW, idR, idP são atribuídos no Passo 4
                    conflitos.push_back(c);

                    cout << "Par conflitante: " << w.thread << " escreve, "
                         << r.thread << " lê em " << variavel
                         << " (sem mutex comum)" << endl;
                }
            }
        }
    }

    // Passo 3: Caso sem conflitos — fórmula vazia
    // Uma fórmula CNF com 0 cláusulas não tem restrições a satisfazer,
    // portanto é trivialmente satisfatível. Porém, do ponto de vista da
    // verificação de race, a ausência de cláusulas significa que não há
    // nenhum par inseguro para o solver investigar.
    // O Módulo 3 (solver) trata "p cnf 0 0" como sinal de UNSAT lógico
    // (sem race) sem nem invocar o MiniSAT, evitando o falso SAT.
    if (conflitos.empty()) {
        cout << "Nenhum par conflitante encontrado. Sem data races." << endl;
        ofstream cnf(filename);
        if (cnf.is_open()) {
            cnf << "p cnf 0 0\n"; // fórmula vazia = sem race a verificar
            cnf.close();
        }
        return conflitos;
    }

    // Passo 4: Atribuir índices proposicionais
    // Cada par conflitante recebe três variáveis proposicionais únicas:
    //   idW → "o acesso de escrita ocorre"
    //   idR → "o acesso de leitura ocorre"
    //   idP → "existe proteção mutex comum"
    // Os índices são contínuos e começam em 1 (exigência do formato DIMACS).
    int nextVar = 1;
    for (auto& c : conflitos) {
        c.idW = nextVar++;
        c.idR = nextVar++;
        c.idP = nextVar++;
    }

    int numVars    = nextVar - 1;
    int numClauses = (int)conflitos.size() * 3; // 3 cláusulas por par

    // Passo 5: Gravar o arquivo DIMACS 
    ofstream cnf(filename);
    if (!cnf.is_open()) {
        cerr << "Erro ao criar " << filename << endl;
        return conflitos;
    }

    // Cabeçalho obrigatório do formato DIMACS
    cnf << "p cnf " << numVars << " " << numClauses << "\n";

    // Uma linha por cláusula, terminada com 0
    for (const auto& c : conflitos) {
        cnf << c.idW << " 0\n";   // W_i deve ser verdadeiro
        cnf << c.idR << " 0\n";   // R_j deve ser verdadeiro
        cnf << "-" << c.idP << " 0\n"; // P deve ser falso (sem proteção)
    }
    cnf.close();

    cout << "CNF gerado em " << filename
         << " (" << numVars << " variáveis, "
         << numClauses << " cláusulas)" << endl;

    return conflitos;
}