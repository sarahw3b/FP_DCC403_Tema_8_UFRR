#include "cnf_gen.h"
#include <iostream>
#include <map>
#include <fstream>

using namespace std;

vector<Conflito> generateCNF(const vector<Access>& acessos, const string& filename) {
    // Passo 1: Agrupar acessos por variável
    map<string, vector<Access>> acessosPorVariavel;
    for (const auto& a : acessos) {
        acessosPorVariavel[a.variavel].push_back(a);
    }

    // Vetor para armazenar os pares conflitantes
    vector<Conflito> conflitos;

    // Passo 2: Para cada variável, identificar pares conflitantes
    for (const auto& par : acessosPorVariavel) {
        const string& variavel = par.first;
        const vector<Access>& accs = par.second;

        // Separar escritas e leituras
        vector<Access> writes, reads;
        for (const auto& a : accs) {
            if (a.tipo == "write") writes.push_back(a);
            else reads.push_back(a);
        }

        // Para cada escrita, procurar leitura de thread diferente
        for (const auto& w : writes) {
            for (const auto& r : reads) {
                // Ignorar se for a mesma thread
                if (w.thread == r.thread) continue;

                // Verificar se há mutex comum
                bool temMutexComum = (!w.mutex.empty() && w.mutex == r.mutex);

                // Se NÃO houver mutex comum, é um par conflitante
                if (!temMutexComum) {
                    Conflito c;
                    c.escrita = w;
                    c.leitura = r;
                    conflitos.push_back(c);
                    
                    cout << "Par conflitante: " << w.thread << " escreve, " 
                         << r.thread << " lê em " << variavel 
                         << " (sem mutex comum)" << endl;
                }
            }
        }
    }

    // Passo 3: Se não houver conflitos, não há data race
    if (conflitos.empty()) {
        cout << "Nenhum par conflitante encontrado. Sem data races." << endl;
        ofstream cnf(filename);
        if (cnf.is_open()) {
            cnf << "p cnf 1 2\n";
            cnf << "1 0\n";
            cnf << "-1 0\n";
            cnf.close();
            cout << "CNF gerado em " << filename << " (UNSAT forçado)" << endl;
        }
        return conflitos;  // Retorna vazio
    }

    // Passo 4: Atribuir IDs às variáveis proposicionais
    int nextVar = 1;
    for (auto& c : conflitos) {
        c.idW = nextVar++;
        c.idR = nextVar++;
        c.idP = nextVar++;
    }

    int numVars = nextVar - 1;
    int numClauses = conflitos.size() * 3;

    // Passo 5: Gerar o arquivo DIMACS
    ofstream cnf(filename);
    if (!cnf.is_open()) {
        cerr << "Erro ao criar " << filename << endl;
        return conflitos;
    }

    cnf << "p cnf " << numVars << " " << numClauses << "\n";

    for (const auto& c : conflitos) {
        cnf << c.idW << " 0\n";
        cnf << c.idR << " 0\n";
        cnf << "-" << c.idP << " 0\n";
    }

    cnf.close();

    cout << "CNF gerado em " << filename << " (" << numVars << " variáveis, " 
         << numClauses << " cláusulas)" << endl;

    return conflitos;
}