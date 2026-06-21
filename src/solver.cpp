/**
 * @file solver.cpp
 * @brief Implementação do Conector com o SAT Solver
 *
 * Decisão de design — verificar cláusulas antes de chamar o solver:
 *   O MiniSAT, quando recebe uma fórmula CNF com 0 cláusulas ("p cnf 0 0"),
 *   responde SAT (uma fórmula vazia é trivialmente satisfatível em lógica
 *   proposicional). Isso causaria um falso positivo: o verificador reportaria
 *   race em código comprovadamente seguro.
 *
 *   A solução adotada é ler o cabeçalho do CNF antes de invocar o solver.
 *   Se o número de cláusulas for zero, retornamos false diretamente, sem
 *   invocar o MiniSAT. Essa verificação custeia O(1) e elimina o falso positivo.
 *
 * Saída do MiniSAT:
 *   O MiniSAT escreve no arquivo de resultado duas linhas quando SAT:
 *     SAT
 *     1 2 -3 0   ← valoração: variáveis positivas = true, negativas = false
 *   E uma linha quando UNSAT:
 *     UNSAT
 *
 *   A comparação usa igualdade exata (line == "SAT") em vez de find()
 *   para evitar que "UNSAT" case com "SAT" por ser substring.
 */

#include "solver.h"
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

bool runSolver(const string& cnfFile, const string& resultFile) {

    // Passo 1: Verificar se há cláusulas no CNF 

    // Lê o cabeçalho do arquivo DIMACS para extrair o número de cláusulas.
    // Pula linhas de comentário (iniciadas por 'c') até encontrar "p cnf N M".
    ifstream cnf(cnfFile);
    if (!cnf.is_open()) {
        cerr << "Erro ao abrir " << cnfFile << endl;
        return false;
    }

    string header;
    getline(cnf, header);
    while (!cnf.eof() && !header.empty() && header[0] == 'c')
        getline(cnf, header);
    cnf.close();

    // Extrai N (variáveis) e M (cláusulas) do cabeçalho "p cnf N M"
    int numVars = 0, numClauses = 0;
    sscanf(header.c_str(), "p cnf %d %d", &numVars, &numClauses);

    // Sem cláusulas = sem pares conflitantes = nenhum race a verificar.
    // Retorna false (UNSAT lógico) sem invocar o MiniSAT.
    if (numClauses == 0)
        return false;

    // Passo 2: Invocar o MiniSAT 

    // Constrói o comando de shell e executa via system().
    // A saída padrão e de erro do MiniSAT são redirecionadas para /dev/null
    // para manter o terminal limpo — apenas o arquivo de resultado importa.
    string cmd = "minisat " + cnfFile + " " + resultFile + " > /dev/null 2>&1";
    system(cmd.c_str());

    // Passo 3: Ler o veredito do arquivo de resultado 
    
    ifstream result(resultFile);
    if (!result.is_open()) {
        cerr << "Erro ao abrir " << resultFile << endl;
        return false;
    }

    string line;
    while (getline(result, line)) {
        // Comparação exata para evitar que "UNSAT" case como "SAT"
        if (line == "UNSAT" || line == "UNSATISFIABLE") return false;
        if (line == "SAT"   || line == "SATISFIABLE")   return true;
    }

    // Se o arquivo estiver vazio ou com formato inesperado, assume seguro
    return false;
}