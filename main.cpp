#include "Solver.h"
#include <iostream>
#include <iomanip>
#include <limits>

// FUNÇÕES AUXILIARES

void exibirCabecalho() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║    OTIMIZACAO DE ROTEIROS TURISTICOS - RIO DE JANEIRO          ║\n";
    std::cout << "║    Problema da Orientacao (Orienteering Problem)               ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

bool obterParametrosUsuario(ParametrosViagem& params) {
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║               CONFIGURACAO DA VIAGEM                           ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
    
    // Latitude
    std::cout << "Latitude de partida (ex: -22.8105 para Galeao): ";
    if (!(std::cin >> params.latitudePartida)) {
        std::cerr << "Latitude invalida.\n";
        return false;
    }
    
    // Longitude
    std::cout << "Longitude de partida (ex: -43.2505 para Galeao): ";
    if (!(std::cin >> params.longitudePartida)) {
        std::cerr << "Longitude invalida.\n";
        return false;
    }
    
    // Orçamento em horas
    std::cout << "Orcamento de tempo em HORAS (ex: 4.5 para 4h30min, 0.5 para 30min): ";
    if (!(std::cin >> params.orcamentoHoras) || params.orcamentoHoras <= 0) {
        std::cerr << "Orcamento invalido. Deve ser maior que zero.\n";
        return false;
    }
    
    // Validação de sanidade (alertar se orçamento parece estranho)
    if (params.orcamentoHoras > 24) {
        std::cout << "ATENCAO: Orçamento de " << params.orcamentoHoras 
                  << " horas (mais de 1 dia). Confirma? (s/n): ";
        char confirmacao;
        std::cin >> confirmacao;
        if (confirmacao != 's' && confirmacao != 'S') {
            std::cout << "Lembre-se: Use 0.5 para 30 minutos, 1.5 para 1h30min, etc.\n";
            return false;
        }
    }
    
    // Velocidade média
    std::cout << "Velocidade media em km/h (ex: 40): ";
    if (!(std::cin >> params.velocidadeKmh) || params.velocidadeKmh <= 0) {
        std::cerr << "Velocidade invalida. Deve ser maior que zero.\n";
        return false;
    }
    
    return true;
}

void exibirResumoParametros(const ParametrosViagem& params) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                  RESUMO DOS PARAMETROS                         ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "  Ponto de partida:  (" << std::fixed << std::setprecision(4) 
              << params.latitudePartida << ", " << params.longitudePartida << ")\n";
    std::cout << "  Orcamento de tempo: " << params.orcamentoHoras << " horas\n";
    std::cout << "  Orcamento de dist.: " << params.orcamentoKm() << " km\n";
    std::cout << "  Velocidade media:   " << params.velocidadeKmh << " km/h\n";
    std::cout << "\n";
}

void compararResultados(const ResultadoSolucao& dp, 
                       const ResultadoSolucao& guloso) 
{
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                  ANALISE COMPARATIVA                           ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
    
    if (!dp.solucaoValida && !guloso.solucaoValida) {
        std::cout << "Nenhuma solucao encontrada por ambos os algoritmos.\n";
        return;
    }
    
    std::cout << "┌─────────────────────────────┬──────────────┬──────────────┐\n";
    std::cout << "│ Metrica                     │ Prog. Dinam. │    Guloso    │\n";
    std::cout << "├─────────────────────────────┼──────────────┼──────────────┤\n";
    
    // Pontuação
    std::cout << "│ Pontuacao                   │ ";
    if (dp.solucaoValida) {
        std::cout << std::setw(12) << dp.pontuacaoTotal;
    } else {
        std::cout << std::setw(12) << "N/A";
    }
    std::cout << " │ ";
    if (guloso.solucaoValida) {
        std::cout << std::setw(12) << guloso.pontuacaoTotal;
    } else {
        std::cout << std::setw(12) << "N/A";
    }
    std::cout << " │\n";
    
    // Distância
    std::cout << "│ Distancia (km)              │ ";
    if (dp.solucaoValida) {
        std::cout << std::fixed << std::setprecision(2) << std::setw(12) << dp.custoKm;
    } else {
        std::cout << std::setw(12) << "N/A";
    }
    std::cout << " │ ";
    if (guloso.solucaoValida) {
        std::cout << std::fixed << std::setprecision(2) << std::setw(12) << guloso.custoKm;
    } else {
        std::cout << std::setw(12) << "N/A";
    }
    std::cout << " │\n";
    
    // Tempo de execução
    std::cout << "│ Tempo execucao (ms)         │ " 
              << std::setw(12) << dp.tempoExecucaoMs << " │ "
              << std::setw(12) << guloso.tempoExecucaoMs << " │\n";
    
    std::cout << "└─────────────────────────────┴──────────────┴──────────────┘\n\n";
    
    // Gap de qualidade
    if (dp.solucaoValida && guloso.solucaoValida && dp.pontuacaoTotal > 0) {
        const double gap = 100.0 * (1.0 - static_cast<double>(guloso.pontuacaoTotal) / 
                                          dp.pontuacaoTotal);
        std::cout << "Gap de qualidade (Guloso vs Otimo): " 
                  << std::fixed << std::setprecision(2) << gap << "%\n";
        
        if (gap < 5.0) {
            std::cout << "   Heuristica gulosa muito proxima do otimo!\n";
        } else if (gap < 15.0) {
            std::cout << "   Heuristica gulosa com boa qualidade.\n";
        } else {
            std::cout << "   Diferenca significativa - considere usar solução otima.\n";
        }
    }
    
    // Speedup
    if (dp.tempoExecucaoMs > 0 && guloso.tempoExecucaoMs > 0) {
        const double speedup = static_cast<double>(dp.tempoExecucaoMs) / 
                              guloso.tempoExecucaoMs;
        std::cout << "\nSpeedup (Guloso e " << std::fixed << std::setprecision(1) 
                  << speedup << "mais rapido)\n";
    }
    
    std::cout << "\n";
}

// FUNÇÃO PRINCIPAL

int main() {
    try {
        exibirCabecalho();
        
        // Inicializar solver
        OrienteeringProblemSolver solver;
        const std::string arquivoCsv = "dados_rio.csv";
        
        // Carregar dados
        std::cout << "Carregando dados de " << arquivoCsv << "...\n";
        solver.carregarDados(arquivoCsv);
        solver.construirGrafo();
        
        // Opcionalmente exibir locais (descomente se necessário)
        // solver.exibirLocais();
        
        // Obter parâmetros do usuário
        ParametrosViagem parametros;
        if (!obterParametrosUsuario(parametros)) {
            std::cerr << "\nErro nos parametros de entrada.\n";
            return 1;
        }
        
        exibirResumoParametros(parametros);
        
        // Executar ambos os algoritmos
        std::cout << "Executando algoritmos...\n";
        
        auto resultadoDP = solver.resolverProgramacaoDinamica(parametros);
        auto resultadoGuloso = solver.resolverGuloso(parametros);
        
        // Exibir resultados
        solver.exibirResultado(resultadoDP, "PROGRAMACAO DINAMICA (OTIMO)");
        solver.exibirResultado(resultadoGuloso, "ALGORITMO GULOSO (HEURISTICA)");
        
        // Comparação
        compararResultados(resultadoDP, resultadoGuloso);
        
        std::cout << "Execucaoo concluIda com sucesso!\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\nERRO CRITICO: " << e.what() << "\n\n";
        return 1;
    }
    
    return 0;
}
