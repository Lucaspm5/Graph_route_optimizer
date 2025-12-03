#include "Solver.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include <iostream>

// PROGRAMAÇÃO DINÂMICA - HELD-KARP COM BITMASK

namespace {
    constexpr double INFINITO = std::numeric_limits<double>::max() / 2;
    constexpr double EPSILON = 1e-9;
}

ResultadoSolucao OrienteeringProblemSolver::resolverProgramacaoDinamica(const ParametrosViagem& params) {
    auto inicioTempo = std::chrono::high_resolution_clock::now();
    
    std::cout << "\nIniciando Programacao Dinamica (Solucao otima)...\n";
    
    ResultadoSolucao resultado;
    
    // Validações
    if (!params.validar()) {
        std::cerr << "Parametros de viagem invalidos.\n";
        return resultado;
    }
    
    validarDados();
    
    const int n = static_cast<int>(locais.size());
    const int numEstados = 1 << n;
    const double orcamentoKm = params.orcamentoKm();
    
    // Tabela DP: dp[mascara][ultimo] = menor custo para visitar os nós 
    // representados pela máscara, terminando no nó 'ultimo'
    std::vector<std::vector<double>> dp(numEstados, 
                                        std::vector<double>(n, INFINITO));
    std::vector<std::vector<int>> predecessor(numEstados, 
                                              std::vector<int>(n, -1));
    
    // CASO BASE: Origem S -> primeiro local
    
    for (int i = 0; i < n; ++i) {
        const double distInicial = calcularDistanciaParaOrigem(i, params);
        
        if (distInicial <= orcamentoKm + EPSILON) {
            dp[1 << i][i] = distInicial;
        }
    }
    
    // TRANSIÇÕES: Held-Karp
    
    for (int mascara = 1; mascara < numEstados; ++mascara) {
        for (int u = 0; u < n; ++u) {
            // u não está na máscara atual
            if (!(mascara & (1 << u))) continue;
            
            const double custoAtual = dp[mascara][u];
            if (custoAtual >= INFINITO) continue;
            
            // Tentar adicionar nó v
            for (int v = 0; v < n; ++v) {
                // v já visitado
                if (mascara & (1 << v)) continue;
                
                const double custoTransicao = distanciasKm[u][v];
                const double novoCusto = custoAtual + custoTransicao;
                
                // Verifica orçamento
                if (novoCusto > orcamentoKm + EPSILON) continue;
                
                const int novaMascara = mascara | (1 << v);
                
                // Atualiza se encontrou caminho melhor
                if (novoCusto < dp[novaMascara][v]) {
                    dp[novaMascara][v] = novoCusto;
                    predecessor[novaMascara][v] = u;
                }
            }
        }
    }
    
    // RECUPERAÇÃO DA MELHOR SOLUÇÃO (incluindo volta para S)
    
    int melhorPontuacao = 0;
    double melhorCustoTotal = INFINITO;
    int melhorMascara = 0;
    int melhorUltimo = -1;
    
    // Primeiro: verificar se É POSSÍVEL chegar em algum local e voltar
    bool existeSolucaoViavel = false;
    for (int i = 0; i < n; ++i) {
        double custoIdaVolta = 2.0 * calcularDistanciaParaOrigem(i, params);
        if (custoIdaVolta <= orcamentoKm + EPSILON) {
            existeSolucaoViavel = true;
            break;
        }
    }
    
    if (!existeSolucaoViavel) {
        std::cout << "Orcamento insuficiente para visitar qualquer local (ida + volta).\n";
        auto fimTempo = std::chrono::high_resolution_clock::now();
        resultado.tempoExecucaoMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            fimTempo - inicioTempo).count();
        return resultado;
    }
    
    for (int mascara = 1; mascara < numEstados; ++mascara) {
        const int pontuacaoAtual = calcularPontuacaoMascara(mascara);
        
        for (int u = 0; u < n; ++u) {
            if (dp[mascara][u] >= INFINITO) continue;
            
            // Adicionar custo de volta para origem
            const double custoVolta = calcularDistanciaParaOrigem(u, params);
            const double custoTotal = dp[mascara][u] + custoVolta;
            
            // VALIDAÇÃO CRÍTICA: Verifica se cabe no orçamento
            if (custoTotal > orcamentoKm + EPSILON) continue;
            
            // Critério de otimização: maximizar pontuação, 
            // desempate por menor custo
            if (pontuacaoAtual > melhorPontuacao ||
                (pontuacaoAtual == melhorPontuacao && 
                 custoTotal < melhorCustoTotal)) 
            {
                melhorPontuacao = pontuacaoAtual;
                melhorCustoTotal = custoTotal;
                melhorMascara = mascara;
                melhorUltimo = u;
            }
        }
    }
    
    // RECONSTRUÇÃO DA ROTA
    
    if (melhorUltimo != -1) {
        std::vector<int> rota;
        int mascara = melhorMascara;
        int atual = melhorUltimo;
        
        while (atual != -1) {
            rota.push_back(atual);
            const int anterior = predecessor[mascara][atual];
            mascara ^= (1 << atual);
            atual = anterior;
        }
        
        std::reverse(rota.begin(), rota.end());
        
        // VALIDAÇÃO FINAL: Verificar se a rota respeita o orçamento
        if (melhorCustoTotal <= orcamentoKm + EPSILON) {
            resultado.rota = rota;
            resultado.pontuacaoTotal = melhorPontuacao;
            resultado.custoKm = melhorCustoTotal;
            resultado.tempoHoras = melhorCustoTotal / params.velocidadeKmh;
            resultado.solucaoValida = true;
        } else {
            std::cout << "Solucao encontrada excede orçamento. Retornando vazio.\n";
        }
    } else {
        std::cout << "Nenhuma rota valida encontrada dentro do orçamento de " 
                  << params.orcamentoHoras << " horas (" << orcamentoKm << " km).\n";
    }
    
    // Tempo de execução
    auto fimTempo = std::chrono::high_resolution_clock::now();
    resultado.tempoExecucaoMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        fimTempo - inicioTempo).count();
    
    return resultado;
}
