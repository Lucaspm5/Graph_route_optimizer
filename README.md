# 🗺️ Otimização de Roteiros Turísticos no Rio de Janeiro

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Paper](https://img.shields.io/badge/paper-available-orange.svg)](SI___paper_trab_final.pdf)

> **Solução inteligente para planejamento de rotas turísticas usando algoritmos de otimização combinatória**

Este projeto implementa e compara duas abordagens para resolver o **Orienteering Problem (OP)** aplicado ao turismo urbano: um método exato baseado em **Programação Dinâmica** (Held-Karp) e uma **Heurística Gulosa** eficiente.

---

## Sumário

- [Sobre o Projeto](#-sobre-o-projeto)
- [Problema](#-problema)
- [Algoritmos](#-algoritmos)
- [Resultados](#-resultados)
- [Estrutura do Projeto](#-estrutura-do-projeto)
- [Referências](#-referências)
- [Autores](#-autores)

---

## Sobre o Projeto

Imagine um turista no Rio de Janeiro com apenas um dia livre. Existem mais de 20 pontos turísticos incríveis para visitar — Cristo Redentor, Pão de Açúcar, Copacabana — mas é impossível conhecer tudo em poucas horas. **Como escolher os melhores locais e otimizar a rota?**

Este projeto resolve exatamente esse desafio usando técnicas avançadas de otimização combinatória.

### Características Principais

- **Solução Ótima Garantida**: Programação Dinâmica encontra a melhor rota possível
- **Solução Ultra-Rápida**: Heurística Gulosa com speedup de 1,8+ milhões de vezes
- **Gap Mínimo**: Diferença média de apenas 4,27% entre as soluções
- **Dados Reais**: 20 pontos turísticos do Rio de Janeiro com coordenadas GPS
- **Configurável**: Ajuste orçamento de tempo, velocidade e ponto de partida

---

## Problema

### Orienteering Problem (OP)

O **Orienteering Problem** modela o desafio de selecionar e ordenar um subconjunto de pontos de interesse para maximizar a atratividade total visitada sem exceder um orçamento de tempo ou distância.

**Definição Formal:**

Dado um grafo **G = (V, E)** onde:
- **V**: Conjunto de locais turísticos
- **E**: Arestas com distâncias entre locais
- **s(v)**: Pontuação (atratividade) de cada local v
- **d(i,j)**: Distância entre locais i e j
- **B**: Orçamento máximo de distância

**Objetivo:**
```
Maximizar: Σ s(v) para cada v visitado
Sujeito a: Σ d(i,j) ≤ B
```

### Por Que É Difícil?

O problema é **NP-difícil**. Com apenas 20 locais, existem:

```
20! = 2.432.902.008.176.640.000 rotas possíveis
```

Testar todas as combinações por força bruta é computacionalmente inviável!

---

## Algoritmos

### Programação Dinâmica (Held-Karp)

**Abordagem:** Exploração exaustiva do espaço de soluções usando bitmask DP.

**Como Funciona:**
- Usa máscaras de bits para representar subconjuntos de locais visitados
- Estado: `dp[máscara][último]` = menor custo para visitar o conjunto representado pela máscara, terminando no local `último`
- Garante encontrar a **solução ótima**

**Complexidade:** O(2ⁿ · n²)

**Exemplo de Estado:**
```cpp
máscara = 00101 → visitou locais 0 e 2
máscara = 11111 → visitou todos os 5 locais
```

**Pseudocódigo:**
```cpp
para cada máscara de 1 até 2^n:
    para cada local u na máscara:
        para cada local v não visitado:
            novoCusto = dp[máscara][u] + distância[u][v]
            se (novoCusto ≤ orçamento):
                novaMáscara = máscara | (1 << v)
                se (novoCusto < dp[novaMáscara][v]):
                    atualizar melhor caminho
```

### Heurística Gulosa

**Abordagem:** Escolha míope baseada na melhor razão benefício/custo.

**Como Funciona:**
- A cada passo, seleciona o local não visitado com maior razão: **pontuação / distância**
- Verifica se cabe no orçamento (incluindo volta para origem)
- Adiciona à rota e repete até nenhum local caber

**Complexidade:** O(n²)

**Critério de Seleção:**
```cpp
razão = pontuação[i] / distância_até_i

Exemplo:
  Local A: 5000 pts, 10 km → razão = 500
  Local B: 3000 pts, 5 km  → razão = 600 ✓ (melhor!)
```

**Pseudocódigo:**
```cpp
rota = [], custoAcumulado = 0
enquanto (existir local viável):
    melhorRazão = -1
    para cada local i não visitado:
        distAteI = distância[atual][i]
        distVolta = distância[i][origem]
        se (custoAcumulado + distAteI + distVolta ≤ orçamento):
            razão = pontuação[i] / distAteI
            se (razão > melhorRazão):
                melhor = i
    adicionar melhor à rota
```

---

## Resultados

### Análise Comparativa (100 Testes Automatizados)

| Métrica | Prog. Dinâmica | Guloso | Diferença |
|---------|----------------|--------|-----------|
| **Pontuação Total** | 54.807,8 pts | 52.804,2 pts | 3,66% |
| **Distância Total** | 115,4 km | 117,2 km | +1,56% |
| **Locais Visitados** | 9,8 locais | 9,4 locais | -4,08% |
| **Tempo de Execução** | 1.878,7 ms | <0,001 ms | **1,8M× mais rápido** |
| **Taxa de Sucesso** | 97% | 97% | 0% |

### Qualidade da Solução

- **Gap Médio**: 4,27%
- **Melhor Caso**: 0,02% (praticamente ótimo)
- **Pior Caso**: 26,81% (orçamento extremamente restrito)
- **Desvio Padrão**: 3,94%

> Em **mais de 80%** dos casos, o gap foi inferior a 7%!

### Eficiência Computacional

A heurística gulosa é **1,8 milhões de vezes mais rápida** que a programação dinâmica, com perda mínima de qualidade.

**Trade-off Ideal:**
- **DP**: Use quando otimalidade é crítica e tempo não é problema
- **Guloso**: Use para aplicações em tempo real (apps mobile, web)

### Análise de Sensibilidade

**Impacto do Orçamento:**
- Orçamentos maiores (4-8h) → gap médio de 2,1%
- Orçamentos reduzidos (0,5-2h) → gap médio de 6,8%

**Impacto do Ponto de Partida:**
- Pontos centrais (Copacabana, Centro) → gap médio de 3,2%
- Pontos periféricos (Pedra da Gávea) → gap médio de 5,8%

### Parâmetros de Viagem

O programa solicitará:

1. **Latitude de Partida**: Ex: `-22.9708` (Copacabana)
2. **Longitude de Partida**: Ex: `-43.1822`
3. **Orçamento de Tempo**: Em horas (Ex: `6.0`)
4. **Velocidade Média**: Em km/h (Ex: `40`)

### Exemplo de Saída

```
==================================================================
RESULTADO: Programação Dinâmica
==================================================================

SOLUÇÃO ENCONTRADA

Pontuação Total:    42.830 pontos
Distância Total:    238.450 km
Tempo Estimado:     5.96 horas (358 minutos)

Rota: S → 3 → 1 → 2 → 8 → 12 → 15 → 18 → 20 → S

Locais visitados (8 locais):
  1. Praia de Copacabana (9589 pts)
  2. Cristo Redentor (5479 pts)
  3. Pão de Açúcar (4110 pts)
  ...

Tempo de execução: 1842 ms
==================================================================
```

---

## Estrutura do Projeto

```
orienteering-rio/
├── src/
│   ├── Data.cpp           # Carregamento de dados e construção do grafo
│   ├── DPSolver.cpp       # Implementação Programação Dinâmica
│   ├── GreedySolver.cpp   # Implementação Heurística Gulosa
│   ├── Solver.h           # Header principal
│   └── main.cpp           # Programa principal
├── data/
│   └── pontos_turisticos_rio.csv  # Dataset com 20 locais
├── docs/
│   ├── SI___paper_trab_final.pdf  # Paper completo
│   └── apresentacao.pdf           # Slides da apresentação
├── tests/
│   └── automated_tests.cpp        # 100 testes automatizados
├── CMakeLists.txt
└── README.md
```

---

## Referências

### Artigos Científicos

1. **Golden, B. L., Levy, L., & Vohra, R.** (1987). *The orienteering problem*. Naval Research Logistics (NRL), 34(3), 307-318.

2. **da Costa, J. V., de Aragao, A. P., & de Lima Veras, N.** (2021). *Aplicação do algoritmo genético para elaboração de roteiros turísticos na Serra da Ibiapaba, Ceará*. ENCompIF.

### Recursos Online

- [USACO Guide - Bitmask DP](https://usaco.guide/gold/dp-bitmasks)
- [Codeforces - DP with Bitmasking](https://codeforces.com/blog/entry/81516)
- [Dataset - Stay Charlie Blog](https://blog.staycharlie.com.br/pontos-turisticos-rio-de-janeiro)

---

## Autores

**Marcos Antônio G. B. Brito**  
Universidade Federal do Piauí (UFPI)  
marcos.brito@ufpi.edu.br

**Válber C. B. Policarpo**  
Universidade Federal do Piauí (UFPI)  
valberufpi@gmail.com

**Lucas Emanuel P. Macêdo Silva**  
Universidade Federal do Piauí (UFPI)  
lucas.macedo@ufpi.edu.br

---

## Licença

Este projeto está sob a licença MIT. Veja o arquivo [LICENSE](LICENSE) para mais detalhes.

---

## Citação

Se você usar este trabalho em sua pesquisa, por favor cite:

```bibtex
@article{brito2024orienteering,
  title={Otimização de Roteiros Turísticos sob Restrições Orçamentárias: 
         Uma Análise Comparativa entre Algoritmos Exatos e Heurísticos 
         Aplicada ao Rio de Janeiro},
  author={Brito, Marcos Antônio GB and Policarpo, Válber CB and 
          Silva, Lucas Emanuel PM},
  journal={Universidade Federal do Piauí},
  year={2024}
}
```

---

## Próximos Passos

- [ ] Visualização interativa das rotas no mapa (integração com Google Maps API)
- [ ] Interface web responsiva
- [ ] Suporte a múltiplos dias de viagem
- [ ] Consideração de horários de funcionamento
- [ ] Preferências personalizadas do usuário
- [ ] Algoritmos híbridos (DP + Guloso)
- [ ] Extensão para outras cidades brasileiras

---

<div align="center">

**⭐ Se este projeto foi útil, considere dar uma estrela!**

</div>
