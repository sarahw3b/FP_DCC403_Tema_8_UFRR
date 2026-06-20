#ifndef CNF_GEN_H
#define CNF_GEN_H

#include "parser.h"
#include <vector>
#include <string>

using namespace std;

struct Conflito {
    Access escrita;
    Access leitura;
    int idW;
    int idR;
    int idP;
};

vector<Conflito> generateCNF(const vector<Access>& acessos, const string& filename);

#endif