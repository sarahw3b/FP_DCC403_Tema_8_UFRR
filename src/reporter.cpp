#include "reporter.h"
#include <iostream>

using namespace std;

void generateReport(const vector<Access>& acessos, 
                    const vector<Conflito>& conflitos,
                    bool hasRace) {
    
    cout << "\n\n- RELATÓRIO DE ANÁLISE: -\n";

    if (!hasRace) {
        cout << "Nenhum data race encontrado.\n";
        cout << "O código está seguro para execução concorrente.\n";
        return;
    }

    cout << "DATA RACE DETECTADO!\n\n";

    // Exibir todos os acessos extraídos
    cout << "ACESSOS EXTRAÍDOS:\n";
    for (const auto& a : acessos) {
        cout << "  Thread: " << a.thread
             << ", Variavel: " << a.variavel
             << ", Tipo: " << a.tipo
             << ", Linha: " << a.linha
             << ", Mutex: " << (a.mutex.empty() ? "nenhum" : a.mutex)
             << endl;
    }

    cout << "\nPARES CONFLITANTES:\n";
    for (const auto& c : conflitos) {
        cout << "  Escrita: " << c.escrita.thread 
             << " (linha " << c.escrita.linha << ")"
             << " em '" << c.escrita.variavel << "'"
             << " | Mutex: " << (c.escrita.mutex.empty() ? "nenhum" : c.escrita.mutex) << "\n";
        
        cout << "  Leitura: " << c.leitura.thread 
             << " (linha " << c.leitura.linha << ")"
             << " em '" << c.leitura.variavel << "'"
             << " | Mutex: " << (c.leitura.mutex.empty() ? "nenhum" : c.leitura.mutex) << "\n";
        
        cout << "  → Nenhum mutex comum protege ambos os acessos.\n\n";
    }

    cout << "SUGESTÃO DE CORREÇÃO:\n";
    cout << "  Utilize o mesmo mutex para proteger ambos os acessos à variável compartilhada.\n";
    cout << "  Exemplo:\n";
    cout << "    pthread_mutex_lock(&mutex);\n";
    cout << "    // acesso à variável\n";
    cout << "    pthread_mutex_unlock(&mutex);\n";
}