#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

using namespace std;

struct Access {
    string thread;        // nome da função thread
    string variavel;      // nome da variável global
    string tipo;          // "read" ou "write"
    int linha;            // número da linha
    string mutex;         // nome do mutex ativo, ou "" se nenhum
};

// Função principal do parser
vector<Access> parseFile(const string& filename);

#endif