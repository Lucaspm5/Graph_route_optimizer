#include "Solver.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// CONSTANTES

namespace {
    constexpr double RAIO_TERRA_KM = 6371.0;
    constexpr int MAX_LOCAIS = 20;
}

// CÁLCULO DE DISTÂNCIA (FÓRMULA DE HAVERSINE)

double OrienteeringProblemSolver::calcularDistanciaHaversine(double lat1, double lon1, double lat2, double lon2) {
    auto grausParaRadianos = [](double graus) { return graus * M_PI / 180.0; };
    
    const double dLat = grausParaRadianos(lat2 - lat1);
    const double dLon = grausParaRadianos(lon2 - lon1);
    
    const double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
                     std::cos(grausParaRadianos(lat1)) * 
                     std::cos(grausParaRadianos(lat2)) *
                     std::sin(dLon / 2) * std::sin(dLon / 2);
    
    const double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    
    return RAIO_TERRA_KM * c;
}

// CARREGAMENTO DE DADOS

void OrienteeringProblemSolver::carregarDados(const std::string& arquivoCsv) {
    std::ifstream arquivo(arquivoCsv);
    
    if (!arquivo.is_open()) {
        throw std::runtime_error("Impossivel abrir o arquivo: " + arquivoCsv);
    }
    
    std::string linha;
    
    // Ignorar cabeçalho
    if (!std::getline(arquivo, linha)) {
        throw std::runtime_error("Arquivo CSV vazio");
    }
    
    locais.clear();
    locais.reserve(MAX_LOCAIS);
    
    int linhaAtual = 2; // Linha 1 é o cabeçalho
    
    while (std::getline(arquivo, linha) && locais.size() < MAX_LOCAIS) {
        std::stringstream ss(linha);
        std::string campo;
        Local local;
        
        try {
            // ID
            if (!std::getline(ss, campo, ',')) continue;
            local.id = std::stoi(campo);
            
            // Nome
            if (!std::getline(ss, campo, ',')) continue;
            local.nome = campo;
            
            // Latitude
            if (!std::getline(ss, campo, ',')) continue;
            local.latitude = std::stod(campo);
            
            // Longitude
            if (!std::getline(ss, campo, ',')) continue;
            local.longitude = std::stod(campo);
            
            // Pontuação
            if (!std::getline(ss, campo, ',')) continue;
            local.pontuacao = std::stoi(campo);
            
            // Validação básica
            if (local.pontuacao < 0) {
                std::cerr << "Aviso: Pontuacao negativa na linha " << linhaAtual << ", ignorando.\n";
                continue;
            }
            
            locais.push_back(local);
            
        } catch (const std::exception& e) {
            std::cerr << "Aviso: Erro ao processar linha " << linhaAtual << " (" << e.what() << "), ignorando.\n";
        }
        
        ++linhaAtual;
    }
    
    if (locais.empty()) {
        throw std::runtime_error("Nenhum local valido foi carregado do CSV");
    }
    
    std::cout << locais.size() << " locais carregados com sucesso.\n";
}

// CONSTRUÇÃO DO GRAFO

void OrienteeringProblemSolver::construirGrafo() {
    validarDados();
    
    const int n = static_cast<int>(locais.size());
    distanciasKm.assign(n, std::vector<double>(n, 0.0));
    
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const double dist = calcularDistanciaHaversine(
                locais[i].latitude, locais[i].longitude,
                locais[j].latitude, locais[j].longitude
            );
            
            distanciasKm[i][j] = distanciasKm[j][i] = dist;  // Grafo simétrico
        }
    }
    
    std::cout << "Grafo construido: " << n << " vertices, " << (n * (n - 1) / 2) << " arestas.\n";
}

// FUNÇÕES AUXILIARES

int OrienteeringProblemSolver::calcularPontuacaoMascara(int mascara) const {
    int pontuacaoTotal = 0;
    const int n = static_cast<int>(locais.size());
    
    for (int i = 0; i < n; ++i) {
        if (mascara & (1 << i)) {
            pontuacaoTotal += locais[i].pontuacao;
        }
    }
    
    return pontuacaoTotal;
}

double OrienteeringProblemSolver::calcularDistanciaParaOrigem(int indiceLocal, const ParametrosViagem& params) const {
    return calcularDistanciaHaversine(
        locais[indiceLocal].latitude,
        locais[indiceLocal].longitude,
        params.latitudePartida,
        params.longitudePartida
    );
}

void OrienteeringProblemSolver::validarDados() const {
    if (locais.empty()) {
        throw std::runtime_error("Nenhum local carregado");
    }
}

// EXIBIÇÃO DE INFORMAÇÕES

void OrienteeringProblemSolver::exibirLocais() const {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "LOCAIS CARREGADOS\n";
    std::cout << std::string(70, '=') << "\n\n";
    
    for (const auto& local : locais) {
        std::cout << std::setw(3) << local.id << " | "
                  << std::left << std::setw(35) << local.nome << " | "
                  << std::right << std::setw(6) << local.pontuacao << " pts | "
                  << std::fixed << std::setprecision(6)
                  << "(" << local.latitude << ", " << local.longitude << ")\n";
    }
    
    std::cout << std::string(70, '=') << "\n";
}

void OrienteeringProblemSolver::exibirResultado(const ResultadoSolucao& resultado, const std::string& nomeAlgoritmo) const {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "RESULTADO: " << nomeAlgoritmo << "\n";
    std::cout << std::string(70, '=') << "\n\n";
    
    if (!resultado.solucaoValida || resultado.rota.empty()) {
        std::cout << "NENHUMA ROTA VALIDA ENCONTRADA\n\n";
        std::cout << "Possiveis causas:\n";
        std::cout << "  • Orcamento muito restrito para visitar qualquer local\n";
        std::cout << "  • Distancia minima (ida + volta) excede o limite disponivel\n";
        std::cout << "  • Sugestao: Aumente o orçamento de tempo ou velocidade media\n";
    } else {
        std::cout << "SOLUCAO ENCONTRADA\n\n";
        std::cout << "Pontuaçao Total:    " << resultado.pontuacaoTotal << " pontos\n";
        std::cout << "Distancia Total:    " << std::fixed << std::setprecision(3) 
                  << resultado.custoKm << " km\n";
        std::cout << "Tempo Estimado:     " << std::fixed << std::setprecision(2) 
                  << resultado.tempoHoras << " horas ("
                  << static_cast<int>(resultado.tempoHoras * 60) << " minutos)\n\n";
        
        std::cout << "Rota: S";
        for (int idx : resultado.rota) {
            std::cout << " → " << locais[idx].id;
        }
        std::cout << " → S\n\n";
        
        std::cout << "Locais visitados (" << resultado.rota.size() << " locais):\n";
        for (size_t i = 0; i < resultado.rota.size(); ++i) {
            const auto& local = locais[resultado.rota[i]];
            std::cout << "  " << (i + 1) << ". " << local.nome 
                     << " (" << local.pontuacao << " pts)\n";
        }
    }
    
    std::cout << "\nTempo de execucao: " << resultado.tempoExecucaoMs << " ms\n";
    std::cout << std::string(70, '=') << "\n";
}
