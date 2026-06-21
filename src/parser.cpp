/**
 * @file parser.cpp
 * @brief Parser Estático
 *
 * Estratégia geral:
 *   O parser não usa uma AST (Árvore Sintática Abstrata) completa nem
 *   depende de compiladores externos. Em vez disso, aplica expressões
 *   regulares (std::regex) sobre o texto do arquivo linha a linha.
 *   Essa abordagem é suficiente para os padrões de código analisados nos
 *   cenários de teste e mantém a ferramenta independente de dependências
 *   externas como libclang.
 *
 * Por que três passagens?
 *   A primeira passagem precisa terminar antes da segunda porque, para
 *   saber se um acesso é a uma variável *global*, precisamos ter a lista
 *   completa de globais. Da mesma forma, para analisar acessos dentro de
 *   uma função de thread, precisamos saber quais funções são threads.
 *   Separar as passagens evita falsos positivos e simplifica o controle
 *   de estado.
 */

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

    // Conjuntos de nomes identificados nas passagens 1 e 2
    set<string> variaveisGlobais;
    set<string> funcoesThread;

    // Pilha de mutexes ativos: empilha no lock, desempilha no unlock.
    // Usar pilha (em vez de uma única string) permite tratar locks aninhados
    // corretamente — o mutex ativo é sempre o do topo da pilha.
    vector<string> mutexStack;

    string currentFunction = ""; // função de thread sendo analisada na passagem 3
    int lineNum = 0;

    // Expressões regulares 

    // Detecta declarações de variável global com tipo primitivo.
    // Exemplos que casam: "int contador = 0;", "double saldo;"
    // pthread_mutex_t é EXCLUÍDO intencionalmente — não é dado compartilhado.
    regex globalVarRegex(
        R"((int|float|double|char|long|short|unsigned|signed)\s+)"
        R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*[=;])"
    );

    // Detecta pthread_create e captura o 3º argumento (função de thread).
    // Exemplo: "pthread_create(&t1, NULL, writer, NULL);" → captura "writer"
    regex createRegex(
        R"(pthread_create\s*\([^,]*,\s*[^,]*,\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*,)"
    );

    // Detecta aquisição de mutex e captura o nome da variável mutex.
    // Exemplo: "pthread_mutex_lock(&meu_mutex);" → captura "meu_mutex"
    regex lockRegex(
        R"(pthread_mutex_lock\s*\(\s*&([a-zA-Z_][a-zA-Z0-9_]*)\s*\))"
    );

    // Detecta liberação de mutex — mesmo padrão do lock.
    regex unlockRegex(
        R"(pthread_mutex_unlock\s*\(\s*&([a-zA-Z_][a-zA-Z0-9_]*)\s*\))"
    );

    // Detecta operações de escrita: atribuição simples e compostas, incremento.
    // Exemplos: "x = 1", "contador += 2", "saldo++", "valor--"
    regex writeRegex(
        R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*(\+\+|--|\ +=|-=|\*=|/=|%=|=))"
    );

    // Detecta referências a variáveis em contextos de leitura (lado direito
    // de expressões, argumentos de funções, condicionais).
    regex readRegex(
        R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*([!=<>]=?|\+|\-|\*|\/|%|\)|,|;|\n))"
    );

    // Leitura do arquivo em memória 

    // Carrega todas as linhas de uma vez para permitir múltiplas passagens
    // sem reabrir o arquivo.
    vector<string> linhas;
    string line;
    while (getline(file, line))
        linhas.push_back(line);

    // PASSAGEM 1: Identificar variáveis globais 
    // Uma variável é global se sua declaração ocorre com balanço de chaves
    // igual a zero, ou seja, fora de qualquer bloco de função.
    // O balanço é atualizado ANTES de verificar cada linha para que uma
    // declaração na mesma linha do '{' de abertura seja tratada corretamente.
    int bracketBalance = 0;
    for (size_t i = 0; i < linhas.size(); i++) {
        const string& l = linhas[i];

        for (char c : l) {
            if (c == '{') bracketBalance++;
            if (c == '}') bracketBalance--;
        }

        if (bracketBalance == 0) {
            smatch match;
            if (regex_search(l, match, globalVarRegex)) {
                string tipo = match[1];
                string nome = match[2];
                // Filtra nomes genéricos de parâmetro e tipo void
                if (nome != "arg" && nome != "args" && tipo != "void") {
                    variaveisGlobais.insert(nome);
                }
            }
        }
    }

    // PASSAGEM 2: Identificar funções de thread 
    // Varre todas as linhas em busca de pthread_create() e coleta o nome
    // da função passada como terceiro argumento.
    for (const string& l : linhas) {
        smatch match;
        if (regex_search(l, match, createRegex))
            funcoesThread.insert(match[1]);
    }

    // PASSAGEM 3: Analisar acessos às variáveis globais 
    // Para cada linha do arquivo:
    //   - Atualiza balanço de chaves para rastrear escopo
    //   - Detecta lock/unlock e gerencia a pilha de mutexes
    //   - Detecta início de função de thread
    //   - Dentro de uma função de thread, registra acessos às globais
    bracketBalance = 0;
    currentFunction = "";
    mutexStack.clear();
    lineNum = 0;

    for (const string& l : linhas) {
        lineNum++;
        smatch match;

        // Atualiza balanço de escopo
        for (char c : l) {
            if (c == '{') bracketBalance++;
            if (c == '}') bracketBalance--;
        }

        // Gerencia pilha de mutexes ativos
        if (regex_search(l, match, lockRegex)) {
            mutexStack.push_back(match[1]); // empilha mutex adquirido
            continue;
        }
        if (regex_search(l, match, unlockRegex)) {
            if (!mutexStack.empty())
                mutexStack.pop_back(); // desempilha mutex liberado
            continue;
        }

        // Detecta entrada em função de thread
        for (const string& func : funcoesThread) {
            if (l.find(func + "(")  != string::npos ||
                l.find(" " + func + "(") != string::npos ||
                l.find("*" + func + "(") != string::npos) {
                currentFunction = func;
                break;
            }
        }

        // Só analisa acessos dentro de funções de thread
        if (currentFunction.empty()) continue;

        // O mutex ativo é o do topo da pilha (suporte a locks aninhados)
        string mutexAtivo = mutexStack.empty() ? "" : mutexStack.back();

        // Detectar ESCRITA 
        if (regex_search(l, match, writeRegex)) {
            string var = match[1];
            if (variaveisGlobais.count(var)) {
                // Evita registrar o mesmo acesso duas vezes na mesma linha
                bool jaCapturado = false;
                for (const auto& a : acessos)
                    if (a.thread == currentFunction && a.variavel == var
                                                    && a.linha == lineNum)
                        { jaCapturado = true; break; }
                if (!jaCapturado)
                    acessos.push_back({currentFunction, var, "write",
                                       lineNum, mutexAtivo});
            }
        }

        // Detectar LEITURA 
        // A busca é iterativa (match.suffix()) porque uma linha pode conter
        // múltiplas referências à mesma ou a diferentes variáveis globais.
        string linhaTemp = l;
        while (regex_search(linhaTemp, match, readRegex)) {
            string var = match[1];

            // Filtra palavras-chave da linguagem e funções de biblioteca
            // para evitar falsos positivos (ex: "return" não é variável)
            static const set<string> keywords = {
                "int","float","double","char","long","short","unsigned","signed",
                "void","NULL","arg","args","return","sizeof","printf","scanf",
                "pthread_create","pthread_join","pthread_mutex_lock",
                "pthread_mutex_unlock"
            };

            if (variaveisGlobais.count(var) && !keywords.count(var)) {
                bool jaCapturado = false;
                for (const auto& a : acessos)
                    if (a.thread == currentFunction && a.variavel == var
                                                    && a.linha == lineNum)
                        { jaCapturado = true; break; }
                if (!jaCapturado)
                    acessos.push_back({currentFunction, var, "read",
                                       lineNum, mutexAtivo});
            }
            linhaTemp = match.suffix().str();
        }

        // Detecta fim da função de thread (linha com apenas '}')
        // e reseta a função atual para interromper a análise de acessos
        if (regex_search(l, regex(R"(^\s*}\s*$)")))
            currentFunction = "";
    }

    return acessos;
}