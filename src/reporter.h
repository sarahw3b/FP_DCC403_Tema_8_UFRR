#ifndef REPORTER_H
#define REPORTER_H

#include "parser.h"
#include "cnf_gen.h"
#include <vector>
#include <string>

using namespace std;

void generateReport(const vector<Access>& acessos, 
                    const vector<Conflito>& conflitos,
                    bool hasRace);

#endif