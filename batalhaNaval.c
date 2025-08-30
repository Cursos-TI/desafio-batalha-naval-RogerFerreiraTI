/**
 * ===========================================
 * BATALHA NAVAL - NÍVEL MESTRE
 * ===========================================
 *
 * Este sistema implementa um jogo de Batalha Naval com funcionalidades avançadas:
 * - Tabuleiro 10x10
 * - Posicionamento de 4 navios (horizontal, vertical e diagonal)
 * - Sistema de habilidades especiais (cone, cruz, octaedro)
 * - Visualização completa do tabuleiro e áreas de impacto
 * - Documentação completa e código manutenível
 *
 * Autor: Roger Ferreira
 * ===========================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================
 * CONSTANTES E DEFINIÇÕES DO SISTEMA
 * ============================================
 */
#define TAMANHO_TABULEIRO 10
#define TAMANHO_HABILIDADE 5
#define MAX_NAVIOS 4
#define MAX_NOME_HABILIDADE 20

// Estados das posições do tabuleiro
#define POSICAO_VAZIA 0
#define POSICAO_NAVIO 3
#define POSICAO_ATINGIDA 2
#define POSICAO_AGUA_ATINGIDA 1

// Estados das áreas de habilidades
#define AREA_NAO_AFETADA 0
#define AREA_AFETADA 1

// Códigos de retorno para operações
#define SUCESSO 1
#define ERRO_POSICAO_INVALIDA 0
#define ERRO_POSICAO_OCUPADA -1
#define ERRO_FORA_LIMITES -2

/*
 * ============================================
 * ESTRUTURAS DE DADOS
 * ============================================
 */

/**
 * Estrutura para representar uma coordenada no tabuleiro
 * Utilizada para posicionamento de navios e aplicação de habilidades
 */
typedef struct {
    int linha;      // Linha no tabuleiro (0-9)
    int coluna;     // Coluna no tabuleiro (A-J)
} Coordenada;

/**
 * Estrutura para representar um navio no jogo
 * Contém todas as informações necessárias para posicionamento
 */
typedef struct {
    Coordenada inicio;      // Posição inicial do navio
    int tamanho;           // Tamanho do navio em células
    char orientacao;       // 'H' = horizontal, 'V' = vertical, 'D' = diagonal
    int id;               // Identificador único do navio
    int foiDestruido;     // Flag para saber se o navio já foi destruído (0 = não, 1 = sim)
} Navio;

/**
 * Estrutura para estatísticas do jogo
 * Mantém controle sobre o estado atual da partida
 */
typedef struct {
    int naviosDestruidos;
    int totalTiros;
    int acertos;
    int erros;
} EstatisticasJogo;

/*
 * ============================================
 * DECLARAÇÃO DE FUNÇÕES (PROTÓTIPOS)
 * ============================================
 */
static void proximaCoordenada(Coordenada* coord, char orientacao);
static inline int coordenadaValida(int linha, int coluna);
void verificarNaviosDestruidos(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO], Navio navios[], int quantidadeNavios, EstatisticasJogo* stats);


/*
 * ============================================
 * FUNÇÕES DE INICIALIZAÇÃO E CONFIGURAÇÃO
 * ============================================
 */

/**
 * Inicializa o tabuleiro com todas as posições vazias
 * Otimizada para performance com loop único
 *
 * @param tabuleiro Matriz do tabuleiro a ser inicializada
 * @complexity O(n²) onde n = TAMANHO_TABULEIRO
 */
void inicializarTabuleiro(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO]) {
    // Utiliza memset para inicialização mais eficiente
    memset(tabuleiro, POSICAO_VAZIA, sizeof(int) * TAMANHO_TABULEIRO * TAMANHO_TABULEIRO);
}

/**
 * Inicializa uma matriz de habilidade com zeros
 * Versão otimizada para melhor performance
 *
 * @param matriz Matriz de habilidade a ser inicializada
 * @complexity O(n²) onde n = TAMANHO_HABILIDADE
 */
void inicializarMatrizHabilidade(int matriz[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE]) {
    memset(matriz, AREA_NAO_AFETADA, sizeof(int) * TAMANHO_HABILIDADE * TAMANHO_HABILIDADE);
}

/**
 * Inicializa as estatísticas do jogo
 *
 * @param stats Ponteiro para a estrutura de estatísticas
 */
void inicializarEstatisticas(EstatisticasJogo* stats) {
    stats->naviosDestruidos = 0;
    stats->totalTiros = 0;
    stats->acertos = 0;
    stats->erros = 0;
}

/*
 * ============================================
 * FUNÇÕES DE VALIDAÇÃO E VERIFICAÇÃO
 * ============================================
 */

/**
 * Verifica se uma coordenada está dentro dos limites do tabuleiro
 * Função auxiliar para evitar acessos inválidos à matriz
 *
 * @param linha Linha a ser verificada
 * @param coluna Coluna a ser verificada
 * @return 1 se válida, 0 caso contrário
 */
static inline int coordenadaValida(int linha, int coluna) {
    return (linha >= 0 && linha < TAMANHO_TABULEIRO &&
            coluna >= 0 && coluna < TAMANHO_TABULEIRO);
}

/**
 * Verifica se uma posição específica está disponível para posicionamento
 *
 * @param tabuleiro Matriz do tabuleiro
 * @param linha Linha a ser verificada
 * @param coluna Coluna a ser verificada
 * @return 1 se disponível, 0 caso contrário
 */
static inline int posicaoDisponivel(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO],
                                   int linha, int coluna) {
    return coordenadaValida(linha, coluna) && tabuleiro[linha][coluna] == POSICAO_VAZIA;
}

/*
 * ============================================
 * FUNÇÕES DE POSICIONAMENTO DE NAVIOS
 * ============================================
 */

/**
 * Calcula a próxima coordenada baseada na orientação do navio
 * Função auxiliar para simplificar o posicionamento
 *
 * @param coord Coordenada atual (será modificada)
 * @param orientacao Orientação do navio
 */
static void proximaCoordenada(Coordenada* coord, char orientacao) {
    switch (orientacao) {
        case 'H': // Horizontal - move para direita
            coord->coluna++;
            break;
        case 'V': // Vertical - move para baixo
            coord->linha++;
            break;
        case 'D': // Diagonal - move diagonal inferior direita
            coord->linha++;
            coord->coluna++;
            break;
        default:
            // Orientação inválida - não move
            break;
    }
}

/**
 * Posiciona um navio no tabuleiro com validação completa
 * Versão otimizada com melhor tratamento de erros
 *
 * @param tabuleiro Matriz do tabuleiro
 * @param navio Estrutura contendo dados do navio
 * @return SUCESSO se bem-sucedido, código de erro caso contrário
 */
int posicionarNavio(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO], Navio navio) {
    Coordenada coord = navio.inicio;
    Coordenada coordenadas[navio.tamanho]; // Armazena coordenadas para rollback se necessário

    // Primeira passada: verifica se todas as posições estão disponíveis
    for (int i = 0; i < navio.tamanho; i++) {
        if (!posicaoDisponivel(tabuleiro, coord.linha, coord.coluna)) {
            if (!coordenadaValida(coord.linha, coord.coluna)) {
                return ERRO_FORA_LIMITES;
            } else {
                return ERRO_POSICAO_OCUPADA;
            }
        }

        coordenadas[i] = coord;
        proximaCoordenada(&coord, navio.orientacao);
    }

    // Segunda passada: posiciona o navio (todas as posições já foram validadas)
    for (int i = 0; i < navio.tamanho; i++) {
        tabuleiro[coordenadas[i].linha][coordenadas[i].coluna] = POSICAO_NAVIO;
    }

    return SUCESSO;
}

/*
 * ============================================
 * FUNÇÕES DE EXIBIÇÃO E INTERFACE
 * ============================================
 */

/**
 * Converte índice de coluna para letra (0->A, 1->B, ..., 9->J)
 *
 * @param coluna Índice da coluna (0-9)
 * @return Caractere correspondente (A-J)
 */
char colunaParaLetra(int coluna) {
    return 'A' + coluna;
}

/**
 * Converte letra para índice de coluna (A->0, B->1, ..., J->9)
 *
 * @param letra Letra da coluna (A-J ou a-j)
 * @return Índice correspondente (0-9) ou -1 se inválida
 */
int letraParaColuna(char letra) {
    if (letra >= 'A' && letra <= 'J') {
        return letra - 'A';
    }
    if (letra >= 'a' && letra <= 'j') {
        return letra - 'a';
    }
    return -1; // Letra inválida
}

/**
 * ---> FUNÇÃO MODIFICADA
 * Exibe o tabuleiro completo com formatação alinhada e legível.
 *
 * @param tabuleiro Matriz do tabuleiro a ser exibida
 */
void exibirTabuleiro(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO]) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║      TABULEIRO DE BATALHA NAVAL      ║\n");
    printf("╚══════════════════════════════════════╝\n");

    // Cabeçalho das colunas (A-J), com 4 espaços de padding para alinhar com as linhas
    printf("    ");
    for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
        printf(" %c ", colunaParaLetra(j)); // Célula com 3 caracteres de largura
    }
    printf("\n");

    // Borda superior, com 3 espaços de padding
    printf("   ┌");
    for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
        printf("───"); // 3 traços por célula
    }
    printf("┐\n");

    // Linhas do tabuleiro (0-9)
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        printf(" %d │", i); // Cabeçalho da linha com 4 caracteres de largura
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            printf(" %d ", tabuleiro[i][j]); // Célula de dados com 3 caracteres
        }
        printf("│\n"); // Borda direita da linha
    }

    // Borda inferior
    printf("   └");
    for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
        printf("───"); // 3 traços por célula
    }
    printf("┘\n");

    printf("\n📋 Legenda:\n");
    printf("   0 = Água (vazio)    3 = Navio\n");
    printf("   1 = Água atingida   2 = Navio atingido\n");
    printf("   Colunas: A-J  |  Linhas: 0-9\n");
}


/**
 * Exibe as coordenadas dos navios posicionados de forma organizada
 * Com sistema de coordenadas A-J e 0-9
 *
 * @param tabuleiro Matriz do tabuleiro
 */
void exibirCoordenadosNavios(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO]) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║       COORDENADAS DOS NAVIOS         ║\n");
    printf("╚══════════════════════════════════════╝\n");

    int contador = 0;
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            if (tabuleiro[i][j] == POSICAO_NAVIO) {
                printf("🚢 Posição do navio: %c%d\n", colunaParaLetra(j), i);
                contador++;
            }
        }
    }
    printf("\n📊 Total de posições ocupadas por navios: %d\n", contador);
}

/*
 * ============================================
 * SISTEMA DE HABILIDADES ESPECIAIS
 * ============================================
 */

/**
 * Cria o padrão de habilidade em cone (pirâmide invertida)
 * Otimizada para melhor performance e legibilidade
 *
 * @param matriz Matriz onde será criado o padrão
 */
void criarHabilidadeCone(int matriz[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE]) {
    inicializarMatrizHabilidade(matriz);

    // Define o padrão cone usando coordenadas específicas
    int padrao[][2] = {
        {0, 2},           // Linha 0: centro
        {1, 1}, {1, 2}, {1, 3},  // Linha 1: 3 posições
        {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}  // Linha 2: linha completa
    };

    int totalPosicoes = sizeof(padrao) / (sizeof(int) * 2);
    for (int i = 0; i < totalPosicoes; i++) {
        matriz[padrao[i][0]][padrao[i][1]] = AREA_AFETADA;
    }
}

/**
 * Cria o padrão de habilidade em cruz
 * Implementação otimizada usando loops direcionados
 *
 * @param matriz Matriz onde será criado o padrão
 */
void criarHabilidadeCruz(int matriz[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE]) {
    inicializarMatrizHabilidade(matriz);

    const int centro = TAMANHO_HABILIDADE / 2;

    // Linha vertical central
    for (int i = 0; i < TAMANHO_HABILIDADE; i++) {
        matriz[i][centro] = AREA_AFETADA;
    }

    // Linha horizontal central
    for (int j = 0; j < TAMANHO_HABILIDADE; j++) {
        matriz[centro][j] = AREA_AFETADA;
    }
}

/**
 * Cria o padrão de habilidade em octaedro (diamante)
 * Padrão otimizado para máximo impacto em área compacta
 *
 * @param matriz Matriz onde será criado o padrão
 */
void criarHabilidadeOctaedro(int matriz[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE]) {
    inicializarMatrizHabilidade(matriz);

    // Padrão diamante centrado
    int padrao[][2] = {
        {0, 2},                    // Topo
        {1, 1}, {1, 2}, {1, 3},   // Meio expandido
        {2, 2}                     // Base
    };

    int totalPosicoes = sizeof(padrao) / (sizeof(int) * 2);
    for (int i = 0; i < totalPosicoes; i++) {
        matriz[padrao[i][0]][padrao[i][1]] = AREA_AFETADA;
    }
}

/**
 * Exibe uma matriz de habilidade com formatação melhorada
 *
 * @param matriz Matriz de habilidade a ser exibida
 * @param nomeHabilidade Nome da habilidade para exibição
 */
void exibirHabilidade(int matriz[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE], const char* nomeHabilidade) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║          HABILIDADE: %-15s ║\n", nomeHabilidade);
    printf("╚══════════════════════════════════════╝\n");

    printf("    ");
    for (int j = 0; j < TAMANHO_HABILIDADE; j++) {
        printf("%2d ", j);
    }
    printf("\n");

    for (int i = 0; i < TAMANHO_HABILIDADE; i++) {
        printf(" %d: ", i);
        for (int j = 0; j < TAMANHO_HABILIDADE; j++) {
            if (matriz[i][j] == AREA_AFETADA) {
                printf(" ● ");  // Posição afetada
            } else {
                printf(" · ");  // Posição não afetada
            }
        }
        printf("\n");
    }
    printf("\n💡 Legenda: ● = Área atingida, · = Área não atingida\n");
}

/*
 * ============================================
 * FUNÇÕES DE ENTRADA DE DADOS DO USUÁRIO
 * ============================================
 */

/**
 * Solicita coordenadas do usuário no formato ColLetra (ex: A5, B3, J9)
 *
 * @param mensagem Mensagem a ser exibida para o usuário
 * @param coord Ponteiro para armazenar a coordenada lida
 * @return 1 se leitura bem-sucedida, 0 caso contrário
 */
int lerCoordenada(const char* mensagem, Coordenada* coord) {
    char entrada[10];
    printf("%s (formato: LetraLinha, ex: A5, B3, J9): ", mensagem);

    if (scanf("%s", entrada) != 1) {
        printf("❌ Erro na leitura. Tente novamente.\n");
        return 0;
    }

    // Verifica se tem pelo menos 2 caracteres
    if (strlen(entrada) < 2) {
        printf("❌ Formato inválido. Use formato LetraLinha (ex: A5).\n");
        return 0;
    }

    // Converte letra para coluna
    int coluna = letraParaColuna(entrada[0]);
    if (coluna == -1) {
        printf("❌ Coluna inválida. Use letras de A a J.\n");
        return 0;
    }

    // Converte número para linha
    char* endptr;
    int linha = strtol(&entrada[1], &endptr, 10);

    // Verifica se a conversão foi bem-sucedida e se a linha é válida
    if (*endptr != '\0' || linha < 0 || linha >= TAMANHO_TABULEIRO) {
        printf("❌ Linha inválida. Use números de 0 a %d.\n", TAMANHO_TABULEIRO - 1);
        return 0;
    }

    coord->linha = linha;
    coord->coluna = coluna;

    printf("✅ Coordenada lida: %c%d (Linha %d, Coluna %d)\n",
           colunaParaLetra(coluna), linha, linha, coluna);

    return 1;
}

/**
 * Solicita coordenadas de ataque ao usuário
 *
 * @param nomeHabilidade Nome da habilidade para exibição
 * @param coord Ponteiro para armazenar a coordenada do ataque
 * @return 1 se bem-sucedido, 0 caso contrário
 */
int lerCoordenadaAtaque(const char* nomeHabilidade, Coordenada* coord) {
    printf("\n🎯 Escolha onde aplicar a habilidade %s:\n", nomeHabilidade);
    return lerCoordenada("Coordenada do centro do ataque", coord);
}

/**
 * Solicita orientação do navio ao usuário
 *
 * @return Caractere da orientação ('H', 'V' ou 'D') ou 0 se inválida
 */
char lerOrientacao() {
    char orientacao;
    printf("Orientação do navio:\n");
    printf("  H - Horizontal (→)\n");
    printf("  V - Vertical (↓)\n");
    printf("  D - Diagonal (↘)\n");
    printf("Escolha (H/V/D): ");

    scanf(" %c", &orientacao);

    // Converte para maiúscula
    if (orientacao >= 'a' && orientacao <= 'z') {
        orientacao = orientacao - 'a' + 'A';
    }

    if (orientacao == 'H' || orientacao == 'V' || orientacao == 'D') {
        printf("✅ Orientação selecionada: %c\n", orientacao);
        return orientacao;
    } else {
        printf("❌ Orientação inválida. Use H, V ou D.\n");
        return 0;
    }
}

/**
 * Solicita ao usuário o posicionamento manual de todos os navios
 *
 * @param tabuleiro Matriz do tabuleiro
 * @param navios Array de navios a serem posicionados
 * @param quantidadeNavios Quantidade de navios a posicionar
 * @return 1 se todos os navios foram posicionados, 0 caso contrário
 */
int posicionarNaviosManualmente(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO],
                               Navio navios[], int quantidadeNavios) {

    // Definição dos tamanhos dos navios
    int tamanhosNavios[] = {4, 3, 3, 2}; // Battleship, Cruiser, Cruiser, Destroyer
    char* nomesNavios[] = {"Battleship", "Cruiser 1", "Cruiser 2", "Destroyer"};

    printf("\n╔════════════════════════════════════════════════╗\n");
    printf("║          POSICIONAMENTO MANUAL DOS NAVIOS      ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    printf("🎯 Você precisa posicionar %d navios no tabuleiro.\n\n", quantidadeNavios);

    // Exibe tabuleiro inicial vazio
    exibirTabuleiro(tabuleiro);

    for (int i = 0; i < quantidadeNavios; i++) {
        printf("\n" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        printf("🚢 POSICIONANDO NAVIO %d: %s (Tamanho: %d)\n", i + 1, nomesNavios[i], tamanhosNavios[i]);
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

        int navioPositionado = 0;
        int tentativas = 0;
        const int maxTentativas = 5;

        while (!navioPositionado && tentativas < maxTentativas) {
            tentativas++;

            Coordenada inicio;
            char orientacao;

            printf("\n🎯 Tentativa %d de %d:\n", tentativas, maxTentativas);

            // Lê coordenada inicial
            if (!lerCoordenada("Digite a posição inicial", &inicio)) {
                printf("⚠️  Tente novamente.\n");
                continue;
            }

            // Lê orientação
            orientacao = lerOrientacao();
            if (orientacao == 0) {
                printf("⚠️  Tente novamente.\n");
                continue;
            }

            // Configura o navio
            navios[i].inicio = inicio;
            navios[i].tamanho = tamanhosNavios[i];
            navios[i].orientacao = orientacao;
            navios[i].id = i + 1;
            navios[i].foiDestruido = 0; // Inicializa o status do navio

            // Tenta posicionar
            int resultado = posicionarNavio(tabuleiro, navios[i]);

            switch (resultado) {
                case SUCESSO:
                    printf("✅ %s posicionado com sucesso em %c%d!\n",
                           nomesNavios[i], colunaParaLetra(inicio.coluna), inicio.linha);
                    navioPositionado = 1;

                    // Exibe tabuleiro atualizado
                    exibirTabuleiro(tabuleiro);
                    break;

                case ERRO_FORA_LIMITES:
                    printf("❌ Erro: O navio sai dos limites do tabuleiro nesta posição!\n");
                    printf("💡 Dica: Considere a direção do navio e o tamanho (%d células).\n", tamanhosNavios[i]);
                    break;

                case ERRO_POSICAO_OCUPADA:
                    printf("❌ Erro: Há outro navio bloqueando esta posição!\n");
                    printf("💡 Dica: Escolha uma área livre no tabuleiro.\n");
                    break;

                default:
                    printf("❌ Erro desconhecido ao posicionar o navio.\n");
                    break;
            }
        }

        if (!navioPositionado) {
            printf("❌ Não foi possível posicionar o %s após %d tentativas.\n", nomesNavios[i], maxTentativas);
            printf("🔄 Reinicie o jogo e tente novamente.\n");
            return 0;
        }
    }

    printf("\n🎉 Todos os navios foram posicionados com sucesso!\n");
    return 1;
}

/*
 * ============================================
 * SISTEMA DE COMBATE E APLICAÇÃO DE HABILIDADES
 * ============================================
 */


/**
 * Verifica todos os navios para ver se algum foi destruído após um ataque.
 * Atualiza as estatísticas e o status do navio se for o caso.
 *
 * @param tabuleiro Matriz do tabuleiro do jogo.
 * @param navios Array com os navios do jogo.
 * @param quantidadeNavios Número total de navios.
 * @param stats Ponteiro para as estatísticas do jogo.
 */
void verificarNaviosDestruidos(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO], Navio navios[], int quantidadeNavios, EstatisticasJogo* stats) {
    for (int i = 0; i < quantidadeNavios; i++) {
        // Pula a verificação se o navio já foi marcado como destruído
        if (navios[i].foiDestruido) {
            continue;
        }

        int partesAtingidas = 0;
        Coordenada coordAtual = navios[i].inicio;

        // Itera por todas as coordenadas que o navio ocupa
        for (int j = 0; j < navios[i].tamanho; j++) {
            if (coordenadaValida(coordAtual.linha, coordAtual.coluna) &&
                tabuleiro[coordAtual.linha][coordAtual.coluna] == POSICAO_ATINGIDA) {
                partesAtingidas++;
            }
            // Calcula a próxima coordenada do navio
            proximaCoordenada(&coordAtual, navios[i].orientacao);
        }

        // Se o número de partes atingidas for igual ao tamanho do navio, ele foi destruído
        if (partesAtingidas == navios[i].tamanho) {
            printf("\n🎉 NAVIO DESTRUÍDO! O navio '%s' foi completamente afundado!\n", (navios[i].id == 1 ? "Battleship" : (navios[i].id <= 3 ? "Cruiser" : "Destroyer")));
            navios[i].foiDestruido = 1; // Marca como destruído para não contar de novo
            stats->naviosDestruidos++;  // Incrementa o contador de estatísticas
        }
    }
}


/**
 * Aplica uma habilidade no tabuleiro em uma coordenada específica
 * Versão otimizada com melhor feedback e controle de erros
 *
 * @param tabuleiro Matriz do tabuleiro
 * @param habilidade Matriz da habilidade a ser aplicada
 * @param centroLinha Linha central onde a habilidade será aplicada
 * @param centroColuna Coluna central onde a habilidade será aplicada
 * @param nomeHabilidade Nome da habilidade para exibição
 * @param navios Array com todos os navios para verificação de destruição
 * @param quantidadeNavios Número total de navios no array
 * @param stats Ponteiro para estatísticas do jogo
 */
void aplicarHabilidadeNoTabuleiro(int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO],
                                  int habilidade[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE],
                                  int centroLinha, int centroColuna,
                                  const char* nomeHabilidade,
                                  Navio navios[], int quantidadeNavios,
                                  EstatisticasJogo* stats) {

    const int deslocamento = TAMANHO_HABILIDADE / 2;
    int acertosNesteTiro = 0;
    int tirosNesteTurno = 0;

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║    APLICANDO HABILIDADE: %-11s ║\n", nomeHabilidade);
    printf("╚══════════════════════════════════════╝\n");
    printf("🎯 Centro do ataque: %c%d\n", colunaParaLetra(centroColuna), centroLinha);
    printf("📍 Posições atingidas:\n\n");

    // Aplica a habilidade com loops otimizados
    for (int i = 0; i < TAMANHO_HABILIDADE; i++) {
        for (int j = 0; j < TAMANHO_HABILIDADE; j++) {
            if (habilidade[i][j] == AREA_AFETADA) {
                int linhaTab = centroLinha - deslocamento + i;
                int colunaTab = centroColuna - deslocamento + j;

                if (coordenadaValida(linhaTab, colunaTab)) {
                    tirosNesteTurno++;
                    printf("   [%c%d] → ", colunaParaLetra(colunaTab), linhaTab);

                    if (tabuleiro[linhaTab][colunaTab] == POSICAO_NAVIO) {
                        printf("💥 ACERTO! Navio atingido!\n");
                        tabuleiro[linhaTab][colunaTab] = POSICAO_ATINGIDA;
                        acertosNesteTiro++;
                    } else if (tabuleiro[linhaTab][colunaTab] == POSICAO_VAZIA) {
                        printf("🌊 Água - Tiro na água\n");
                        tabuleiro[linhaTab][colunaTab] = POSICAO_AGUA_ATINGIDA;
                    } else if (tabuleiro[linhaTab][colunaTab] == POSICAO_ATINGIDA) {
                        printf("🔄 Já atingido anteriormente\n");
                    } else {
                        printf("🌊 Água já atingida\n");
                    }
                }
            }
        }
    }

    // Verifica se algum navio foi destruído após a rodada de ataques
    if (acertosNesteTiro > 0) {
        verificarNaviosDestruidos(tabuleiro, navios, quantidadeNavios, stats);
    }

    // Atualiza estatísticas se fornecidas
    if (stats != NULL) {
        stats->totalTiros += tirosNesteTurno;
        stats->acertos += acertosNesteTiro;
        stats->erros += (tirosNesteTurno - acertosNesteTiro);
    }

    printf("\n📊 Resultado deste ataque:\n");
    printf("   • Tiros disparados: %d\n", tirosNesteTurno);
    printf("   • Acertos: %d\n", acertosNesteTiro);
    printf("   • Erros: %d\n", tirosNesteTurno - acertosNesteTiro);
    if (acertosNesteTiro > 0 && tirosNesteTurno > 0) {
        printf("   🎉 Taxa de acerto: %.1f%%\n", (float)acertosNesteTiro / tirosNesteTurno * 100);
    }
}

/**
 * Exibe estatísticas finais do jogo
 *
 * @param stats Estrutura com as estatísticas
 */
void exibirEstatisticasFinais(EstatisticasJogo* stats) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║        ESTATÍSTICAS FINAIS           ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("📊 Total de tiros disparados: %d\n", stats->totalTiros);
    printf("🎯 Total de acertos: %d\n", stats->acertos);
    printf("❌ Total de erros: %d\n", stats->erros);
    if (stats->totalTiros > 0) {
        printf("📈 Taxa de acerto geral: %.1f%%\n", (float)stats->acertos / stats->totalTiros * 100);
    }
    printf("🚢 Navios destruídos: %d de %d\n", stats->naviosDestruidos, MAX_NAVIOS);
}

/*
 * ============================================
 * FUNÇÃO PRINCIPAL DO SISTEMA
 * ============================================
 */

/**
 * Função principal do sistema
 * Controla todo o fluxo do jogo de batalha naval
 *
 * @return 0 se execução bem-sucedida
 */
int main() {
    // Declaração das estruturas principais
    int tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];
    int habilidadeCone[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE];
    int habilidadeCruz[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE];
    int habilidadeOctaedro[TAMANHO_HABILIDADE][TAMANHO_HABILIDADE];
    EstatisticasJogo stats;

    // Banner inicial do sistema
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║          BATALHA NAVAL - NÍVEL MESTRE          ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    printf("🎮 Iniciando simulação completa do jogo...\n");

    // Inicialização do sistema
    inicializarTabuleiro(tabuleiro);
    inicializarEstatisticas(&stats);

    /*
     * POSICIONAMENTO MANUAL DOS NAVIOS
     * Entrada de dados interativa conforme requisito
     */
    printf("\n🎮 Bem-vindo ao modo de posicionamento manual!\n");
    printf("📋 Instruções:\n");
    printf("   • Use coordenadas no formato LetraLinha (ex: A5, B3, J9)\n");
    printf("   • Colunas: A até J\n");
    printf("   • Linhas: 0 até 9\n");
    printf("   • Orientações: H (horizontal), V (vertical), D (diagonal)\n\n");

    // Array de navios para posicionamento manual
    Navio navios[MAX_NAVIOS];

    // Chama função para posicionamento manual
    if (!posicionarNaviosManualmente(tabuleiro, navios, MAX_NAVIOS)) {
        printf("❌ Falha no posicionamento dos navios. Encerrando jogo.\n");
        return 1;
    }

    // Exibição do tabuleiro inicial
    exibirTabuleiro(tabuleiro);
    exibirCoordenadosNavios(tabuleiro);

    // Inicialização do sistema de habilidades
    printf("\n╔════════════════════════════════════════════════╗\n");
    printf("║           SISTEMA DE HABILIDADES ESPECIAIS     ║\n");
    printf("╚════════════════════════════════════════════════╝\n");

    criarHabilidadeCone(habilidadeCone);
    exibirHabilidade(habilidadeCone, "CONE");

    criarHabilidadeCruz(habilidadeCruz);
    exibirHabilidade(habilidadeCruz, "CRUZ");

    criarHabilidadeOctaedro(habilidadeOctaedro);
    exibirHabilidade(habilidadeOctaedro, "OCTAEDRO");

    // Simulação de combate
    printf("\n╔════════════════════════════════════════════════╗\n");
    printf("║               INÍCIO DO COMBATE                ║\n");
    printf("╚════════════════════════════════════════════════╝\n");

    printf("🎯 Agora você pode atacar usando habilidades especiais!\n");
    printf("📚 Habilidades disponíveis:\n");
    printf("   1. CONE - Ataque em forma de cone\n");
    printf("   2. CRUZ - Ataque em cruz\n");
    printf("   3. OCTAEDRO - Ataque em diamante\n\n");

    // Coordenadas de ataque solicitadas ao usuário
    Coordenada ataques[3];

    // Ataque com CONE
    if (lerCoordenadaAtaque("CONE", &ataques[0])) {
        aplicarHabilidadeNoTabuleiro(tabuleiro, habilidadeCone, ataques[0].linha, ataques[0].coluna, "CONE", navios, MAX_NAVIOS, &stats);
    }

    // Ataque com CRUZ
    if (lerCoordenadaAtaque("CRUZ", &ataques[1])) {
        aplicarHabilidadeNoTabuleiro(tabuleiro, habilidadeCruz, ataques[1].linha, ataques[1].coluna, "CRUZ", navios, MAX_NAVIOS, &stats);
    }

    // Ataque com OCTAEDRO
    if (lerCoordenadaAtaque("OCTAEDRO", &ataques[2])) {
        aplicarHabilidadeNoTabuleiro(tabuleiro, habilidadeOctaedro, ataques[2].linha, ataques[2].coluna, "OCTAEDRO", navios, MAX_NAVIOS, &stats);
    }

    // Exibição do tabuleiro final
    printf("\n╔════════════════════════════════════════════════╗\n");
    printf("║              TABULEIRO FINAL                   ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    exibirTabuleiro(tabuleiro);

    // Estatísticas finais
    exibirEstatisticasFinais(&stats);

    printf("\n╔════════════════════════════════════════════════╗\n");
    printf("║                FIM DA SIMULAÇÃO                ║\n");
    printf("║            Obrigado por jogar!                 ║\n");
    printf("╚════════════════════════════════════════════════╝\n");

    return 0;
}