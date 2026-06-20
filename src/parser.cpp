#include "parser.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <set>

using namespace std;

vector<Access> parseFile(const string& filename) {
    vector<Access> acessos;
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Erro: não foi possível abrir " << filename << endl;
        return acessos;
    }

    set<string> variaveisGlobais;
    set<string> funcoesThread;
    vector<string> mutexStack;
    string currentFunction = "";
    int lineNum = 0;
    string line;

    // Regex para detectar padrões
    regex globalVarRegex(R"((int|float|double|char|long|short|unsigned|signed|void|pthread_mutex_t)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*[=;])");
    regex createRegex(R"(pthread_create\s*\([^,]*,\s*[^,]*,\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*,)");
    regex lockRegex(R"(pthread_mutex_lock\s*\(\s*&([a-zA-Z_][a-zA-Z0-9_]*)\s*\))");
    regex unlockRegex(R"(pthread_mutex_unlock\s*\(\s*&([a-zA-Z_][a-zA-Z0-9_]*)\s*\))");
    
    // Regex para escrita: captura variável seguida por =, +=, -=, ++, --
    regex writeRegex(R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*(\+\+|--|\+=|-=|\*=|/=|%=|=))");
    
    // Regex para leitura: captura variável em contextos de leitura
    regex readRegex(R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*([!=<>]=?|\+|\-|\*|\/|%|\)|,|;|\n))");

    // Primeira passagem: ler TODO o arquivo em um vetor de linhas
    vector<string> linhas;
    while (getline(file, line)) {
        linhas.push_back(line);
    }

    // Identificar variáveis globais: apenas declarações que NÃO estão dentro de funções
    // Estratégia: procurar por "tipo nome =" ou "tipo nome;" que não estão precedidos por ")" 
    // ou que estão no escopo global (antes de main ou fora de funções)
    int bracketBalance = 0;
    for (size_t i = 0; i < linhas.size(); i++) {
        string l = linhas[i];
        
        // Atualizar balanço de chaves ANTES de verificar a linha
        for (char c : l) {
            if (c == '{') bracketBalance++;
            if (c == '}') bracketBalance--;
        }
        
        // Só detecta global se NÃO estiver dentro de uma função (bracketBalance == 0)
        if (bracketBalance == 0) {
            smatch match;
            if (regex_search(l, match, globalVarRegex)) {
                string tipo = match[1];
                string nome = match[2];
                if (nome != "arg" && nome != "args" && tipo != "void") {
                    variaveisGlobais.insert(nome);
                }
            }
        }
    }

    // Identificar funções thread (pthread_create)
    for (const string& l : linhas) {
        smatch match;
        if (regex_search(l, match, createRegex)) {
            string func = match[1];
            funcoesThread.insert(func);
        }
    }

    // Segunda passagem: analisar acessos
    bracketBalance = 0;
    currentFunction = "";
    mutexStack.clear();
    lineNum = 0;

    for (const string& l : linhas) {
        lineNum++;
        string line = l;
        smatch match;

        // Atualizar balanço de chaves
        for (char c : line) {
            if (c == '{') bracketBalance++;
            if (c == '}') bracketBalance--;
        }

        // Detectar lock
        if (regex_search(line, match, lockRegex)) {
            string mutex = match[1];
            mutexStack.push_back(mutex);
            continue;
        }

        // Detectar unlock
        if (regex_search(line, match, unlockRegex)) {
            if (!mutexStack.empty()) {
                mutexStack.pop_back();
            }
            continue;
        }

        // Detectar início de uma função que é thread
        for (const string& func : funcoesThread) {
            if (line.find(func + "(") != string::npos || 
                line.find(" " + func + "(") != string::npos ||
                line.find("*" + func + "(") != string::npos) {
                currentFunction = func;
                break;
            }
        }

        // Se não estamos dentro de uma thread, pula
        if (currentFunction.empty()) continue;

        // --- DETECTAR ESCRITA ---
        if (regex_search(line, match, writeRegex)) {
            string var = match[1];
            // Só processa se for uma variável global conhecida
            if (variaveisGlobais.find(var) != variaveisGlobais.end()) {
                string mutexAtivo = mutexStack.empty() ? "" : mutexStack.back();
                // Evitar duplicata
                bool jaCapturado = false;
                for (const auto& a : acessos) {
                    if (a.thread == currentFunction && a.variavel == var && a.linha == lineNum) {
                        jaCapturado = true;
                        break;
                    }
                }
                if (!jaCapturado) {
                    acessos.push_back({currentFunction, var, "write", lineNum, mutexAtivo});
                }
            }
        }

        // --- DETECTAR LEITURA ---
        string linhaTemp = line;
        while (regex_search(linhaTemp, match, readRegex)) {
            string var = match[1];
            // Só processa se for uma variável global conhecida
            // E NÃO for um tipo primitivo ou palavra-chave
            if (variaveisGlobais.find(var) != variaveisGlobais.end() &&
                var != "int" && var != "float" && var != "double" && var != "char" &&
                var != "long" && var != "short" && var != "unsigned" && var != "signed" &&
                var != "void" && var != "NULL" && var != "arg" && var != "args" &&
                var != "return" && var != "sizeof" && var != "printf" && var != "scanf" &&
                var != "pthread_create" && var != "pthread_join" && var != "pthread_mutex_lock" &&
                var != "pthread_mutex_unlock") {
                bool jaCapturado = false;
                for (const auto& a : acessos) {
                    if (a.thread == currentFunction && a.variavel == var && a.linha == lineNum) {
                        jaCapturado = true;
                        break;
                    }
                }
                if (!jaCapturado) {
                    string mutexAtivo = mutexStack.empty() ? "" : mutexStack.back();
                    acessos.push_back({currentFunction, var, "read", lineNum, mutexAtivo});
                }
            }
            linhaTemp = match.suffix().str();
        }

        // Resetar currentFunction quando encontrar o fim da função
        if (regex_search(line, regex(R"(^\s*}\s*$)"))) {
            currentFunction = "";
        }
    }

    return acessos;
}