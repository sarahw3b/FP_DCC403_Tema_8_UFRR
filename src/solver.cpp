#include "solver.h"
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

bool runSolver(const string& cnfFile, const string& resultFile) {
    // 1. Executar o MiniSAT
    string cmd = "minisat " + cnfFile + " " + resultFile + " > /dev/null 2>&1";
    int ret = system(cmd.c_str());

    // 2. Abrir o arquivo de resultado
    ifstream result(resultFile);
    if (!result.is_open()) {
        cerr << "Erro ao abrir " << resultFile << endl;
        return false;
    }

    // 3. Procurar por UNSAT primeiro (prioridade)
    string line;
    while (getline(result, line)) {
        // Primeiro, verifica se é UNSAT (para não confundir com SAT)
        if (line.find("UNSAT") != string::npos) {
            return false;  // UNSAT → sem data race
        }
        // Depois, verifica se é SAT
        if (line.find("SAT") != string::npos) {
            return true;   // SAT → data race
        }
    }

    // Se não encontrar nada, assumir UNSAT
    return false;
}