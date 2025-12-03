#ifndef SOLVER_H
#define SOLVER_H

#include <string>
#include <vector>
#include <optional>
#include <memory>

// ESTRUTURAS DE DADOS

struct Local {
    int id;
    std::string nome;
    double latitude;
    double longitude;
    int pontuacao;
    
    Local() : id(0), latitude(0.0), longitude(0.0), pontuacao(0) {}
};

struct ParametrosViagem {
    double latitudePartida;
    double longitudePartida;
    double orcamentoHoras;
    double velocidadeKmh;
    
    bool validar() const {
        return velocidadeKmh > 0.0 && orcamentoHoras > 0.0;
    }
    
    double orcamentoKm() const {
        return orcamentoHoras * velocidadeKmh;
    }
};

struct ResultadoSolucao {
    int pontuacaoTotal;
    double custoKm;
    double tempoHoras;
    std::vector<int> rota;  // Índices dos locais visitados
    long tempoExecucaoMs;
    bool solucaoValida;
    
    ResultadoSolucao() : pontuacaoTotal(0), custoKm(0.0), tempoHoras(0.0), tempoExecucaoMs(0), solucaoValida(false) {}
};

// CLASSE PRINCIPAL

class OrienteeringProblemSolver {
	public:
	    OrienteeringProblemSolver() = default;
	    
	    // Carregamento de dados
	    void carregarDados(const std::string& arquivoCsv);
	    void construirGrafo();
	    
	    // Algoritmos de solução
	    ResultadoSolucao resolverProgramacaoDinamica(const ParametrosViagem& params);
	    ResultadoSolucao resolverGuloso(const ParametrosViagem& params);
	    
	    // Utilitários
	    void exibirLocais() const;
	    void exibirResultado(const ResultadoSolucao& resultado, const std::string& nomeAlgoritmo) const;
	    
	    int quantidadeLocais() const { return static_cast<int>(locais.size()); }

	private:
	    std::vector<Local> locais;
	    std::vector<std::vector<double>> distanciasKm;  // Matriz de distâncias
	    
	    // Cálculos geométricos
	    static double calcularDistanciaHaversine(double lat1, double lon1, double lat2, double lon2);
	    
	    // Auxiliares para DP
	    int calcularPontuacaoMascara(int mascara) const;
	    double calcularDistanciaParaOrigem(int indiceLocal, const ParametrosViagem& params) const;
	    
	    // Validação
	    void validarDados() const;
};

#endif // SOLVER_H
