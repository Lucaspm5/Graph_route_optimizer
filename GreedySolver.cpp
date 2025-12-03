#include "Solver.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>

// ALGORITMO GULOSO (HEURÍSTICA)

namespace {
    constexpr double EPSILON = 1e-9;
    
    struct CandidatoGuloso {
        int indice;
        double razaoBeneficio;
        double distanciaKm;
        
        bool operator>(const CandidatoGuloso& outro) const {
            return razaoBeneficio > outro.razaoBeneficio;
        }
    };
}

ResultadoSolucao OrienteeringProblemSolver::resolverGuloso(
    const ParametrosViagem& params) 
{
    auto inicioTempo = std::chrono::high_resolution_clock::now();
    
    std::cout << "\nIniciando Algoritmo Guloso (Heuristica Rapida)...\n";
    
    ResultadoSolucao resultado;
    
    // Validações
    if (!params.validar()) {
        std::cerr << "Parametros de viagem invalidos.\n";
        return resultado;
    }
    
    validarDados();
    
    const int n = static_cast<int>(locais.size());
    const double orcamentoKm = params.orcamentoKm();
    
    // Verificar viabilidade: é possível visitar pelo menos 1 local?
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
    
    std::vector<bool> visitado(n, false);
    std::vector<int> rota;
    rota.reserve(n);
    
    int pontuacaoTotal = 0;
    double custoAcumuladoKm = 0.0;
    int localAtual = -1;  // -1 representa a origem S
    
    // LOOP GULOSO: Escolher próximo local com melhor razão 
    // pontuação/distância
    
    while (true) {
        CandidatoGuloso melhorCandidato{-1, -1.0, 0.0};
        
        // Avaliar todos os locais não visitados
        for (int i = 0; i < n; ++i) {
            if (visitado[i]) continue;
            
            // Distância do local atual até i
            double distAtei;
            if (localAtual == -1) {
                distAtei = calcularDistanciaParaOrigem(i, params);
            } else {
                distAtei = distanciasKm[localAtual][i];
            }
            
            // Distância de i de volta para origem
            const double distVolta = calcularDistanciaParaOrigem(i, params);
            
            // CRÍTICO: Verifica se consegue ir até i E voltar para S
            const double custoTotalSeEscolherI = custoAcumuladoKm + distAtei + distVolta;
            
            if (custoTotalSeEscolherI > orcamentoKm + EPSILON) {
                continue;  // Não cabe no orçamento
            }
            
            // Critério guloso: pontuação / distância até chegar em i
            const double razao = static_cast<double>(locais[i].pontuacao) / 
                                (distAtei + EPSILON);
            
            if (razao > melhorCandidato.razaoBeneficio) {
                melhorCandidato.indice = i;
                melhorCandidato.razaoBeneficio = razao;
                melhorCandidato.distanciaKm = distAtei;
            }
        }
        
        // Nenhum candidato viável encontrado
        if (melhorCandidato.indice == -1) {
            break;
        }
        
        // Adicionar melhor candidato à rota
        const int proximo = melhorCandidato.indice;
        visitado[proximo] = true;
        rota.push_back(proximo);
        custoAcumuladoKm += melhorCandidato.distanciaKm;
        pontuacaoTotal += locais[proximo].pontuacao;
        localAtual = proximo;
    }
    
    // ADICIONAR VOLTA PARA ORIGEM E VALIDAÇÃO FINAL
    
    if (!rota.empty()) {
        const int ultimoLocal = rota.back();
        const double distVolta = calcularDistanciaParaOrigem(ultimoLocal, params);
        const double custoTotalFinal = custoAcumuladoKm + distVolta;
        
        // VALIDAÇÃO CRÍTICA: Verificar se a rota completa respeita orçamento
        if (custoTotalFinal <= orcamentoKm + EPSILON) {
            resultado.rota = rota;
            resultado.pontuacaoTotal = pontuacaoTotal;
            resultado.custoKm = custoTotalFinal;
            resultado.tempoHoras = custoTotalFinal / params.velocidadeKmh;
            resultado.solucaoValida = true;
        } else {
            // Isso não deveria acontecer devido às verificações anteriores
            std::cerr << "AVISO: Rota construida excede orcamento!\n";
            std::cerr << "    Custo: " << custoTotalFinal << " km, Limite: " << orcamentoKm << " km\n";
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
