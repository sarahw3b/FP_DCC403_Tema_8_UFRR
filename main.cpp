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

    vector<Access> acessos = parseFile(argv[1]);

    cout << "\nACESSOS EXTRAÍDOS:\n\n";
    for (const auto& a : acessos) {
        cout << "Thread: " << a.thread
             << ", Variavel: " << a.variavel
             << ", Tipo: " << a.tipo
             << ", Linha: " << a.linha
             << ", Mutex: " << (a.mutex.empty() ? "nenhum" : a.mutex)
             << endl;
    }
    vector<Conflito> conflitos = generateCNF(acessos, "output.cnf");

    bool hasRace = runSolver("output.cnf", "resultado.txt");

    cout << "\n\nRESULTADO:\n";
    if (hasRace) {
        cout << "DATA RACE DETECTADO!" << endl;
    } else {
        cout << "Nenhum data race encontrado." << endl;
    }

    generateReport(acessos, conflitos, hasRace);

    return 0;
}