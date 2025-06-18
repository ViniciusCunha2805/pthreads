#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma comment(lib,"pthreadVC2.lib")
#define HAVE_STRUCT_TIMESPEC

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

// DEFINI��ES GLOBAIS
#define LARGURA_MX 20000
#define ALTURA_MX 20000
#define ALTURA_MACROBLOCO 1000
#define LARGURA_MACROBLOCO 1000
#define NUM_THREADS 8
#define SEED 500

// Macrobloco com:
//                  proximaLinha --> Proxima linha a ser atribuida para um macrobloco 
//                  proximaColuna --> Proxima coluna a ser atribuida para um macrobloco
//                  
//                  !!!linha e coluna (regiao) ainda nao verificada(s) da matriz!!!!
typedef struct {
    int proximaLinha;
    int proximaColuna;
} MacroBloco;

// VARIAVEIS GLOBAIS
int** mx;
int qtdPrimos = 0;
//pthread_mutex_t mutexQtdPrimo;
//pthread_mutex_t mutexMacrobloco;
MacroBloco mcGlobal = { 0, 0 };


// FUNCOES
void inicializador();
void Alocar_matriz_real();
void Liberar_matriz_real();
void preencheMatriz();
void buscaSequencial();
void buscaParalela();
void* processaMacrobloco();
int ehPrimo(int n);


void buscaParalela() {
    float tempoInicial = clock();

    pthread_t threads[NUM_THREADS];
    // o init permite que o mutex seja usado mesmo que dentro de alguma funcao chamada
    //pthread_mutex_init(&mutexQtdPrimo, NULL);
    //pthread_mutex_init(&mutexMacrobloco, NULL);

    //criacaoo e execução paralela das threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, processaMacrobloco, NULL);
    }

    // apos os threads serem processados paralelamente acima, o join aguarda o retorno (respostas) de todos os threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    //finaliza e libera o mutex
    //pthread_mutex_destroy(&mutexQtdPrimo);
    //pthread_mutex_destroy(&mutexMacrobloco);

    float tempoFinal = clock();
    float tempoTotal = (tempoFinal - tempoInicial) / CLOCKS_PER_SEC;
    printf("Existem %d primos na matriz.\n", qtdPrimos);
    printf("Tempo gasto durante a busca paralela: %f segundos\n", tempoTotal);

    // Reseta o macrobloco por conta do menu...
    mcGlobal.proximaLinha = 0;
    mcGlobal.proximaColuna = 0;
}

// Faz o tratamento de duas regiões críticas, sendo elas: Incrementação do macrobloco (proxima linha e coluna) e
//                                                        Incrementação da variável de números primos global, utilizando uma local (qtdPrimosLocal) para armazenar o valor final (após percorrer todo macrobloco).
void* processaMacrobloco() {

    // FLAG PARA VERIficar se ainda tem macroblocos
    int terminouMacroblocos = 0;
    // LOOP para a thread continuar processando os Macroblocos ate nao acabar.
    while (!terminouMacroblocos) {

        // Variavel para salvar os indices da linha e coluna para atribui-los a uma thread
        int linhaAtual, colunaAtual;

        // Pega o proximo bloco disponivel
        //mutex para tratar a regiao critica do bloco global
        //pthread_mutex_lock(&mutexMacrobloco);
        // se o macrobloco exceder o tamanho da matriz finaliza programa
        if (mcGlobal.proximaLinha >= ALTURA_MX) {
            // Free da thread (liberacao dos recursos) e desbloqueio do mutex
            //pthread_mutex_unlock(&mutexMacrobloco);
            terminouMacroblocos = 1;
        }
        // senao atribui proxima linha e coluna para ser verificada
        // Armazena a linha e coluna nao verificadas da matriz (proxima linha/coluna do macrobloco) para verificação por essa thread...
        linhaAtual = mcGlobal.proximaLinha;
        colunaAtual = mcGlobal.proximaColuna;

        // Incrementa a coluna do macrobloco para a proxima coluna da matriz que precisa ser verificada.
        mcGlobal.proximaColuna += LARGURA_MACROBLOCO;
        // se proxima coluna exceder a largura da matriz, pula para proxima linha
        if (mcGlobal.proximaColuna >= LARGURA_MX) {
            mcGlobal.proximaColuna = 0;
            // Incrementa a linha do macrobloco para a proxima linha da matriz que precisa ser verificada, já que não tem mais colunas nesta linha para ser verificada.
            mcGlobal.proximaLinha += ALTURA_MACROBLOCO;
        }

        // libera mutex quanado o macrobloco avancar uma coluna ou linha da matriz (apos a modificacao do macrobloco)
        //pthread_mutex_unlock(&mutexMacrobloco);

        // Loop de verificação da determinada região da matriz (macrobloco) atribuida a esta thread.
        // Variavel local contador de primos no macrobloco para incrementacao na variavel global qtdPrimos
        int qtdPrimosLocal = 0;
        for (int i = linhaAtual; i < linhaAtual + ALTURA_MACROBLOCO && i < ALTURA_MX; i++) {
            for (int j = colunaAtual; j < colunaAtual + LARGURA_MACROBLOCO && j < LARGURA_MX; j++) {
                if (ehPrimo(mx[i][j])) {
                    qtdPrimosLocal++;
                }
            }
        }

        // Mutex para modicar o valor da regiao critica (qtdPrimos).
        //pthread_mutex_lock(&mutexQtdPrimo);
        qtdPrimos += qtdPrimosLocal;
        //pthread_mutex_unlock(&mutexQtdPrimo);

    }

    // Free da thread (liberacao dos recursos)
    pthread_exit(NULL);
}



void buscaSequencial() {
    float tempoInicial = clock();

    for (int l = 0; l < ALTURA_MX; l++) {
        for (int c = 0; c < LARGURA_MX; c++) {
            if (ehPrimo(mx[l][c])) {
                qtdPrimos++;
            }
        }
    }

    float tempoFinal = clock();
    float tempoTotal = (tempoFinal - tempoInicial) / CLOCKS_PER_SEC;

    printf("Existem %d primos na matriz.\n", qtdPrimos);
    printf("Tempo gasto na busca sequencial: %f segundos\n", tempoTotal);
}

void Alocar_matriz_real() {
    mx = calloc(ALTURA_MX, sizeof(int*));
    if (mx == NULL) {
        printf("Erro: memoria insuficiente\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ALTURA_MX; i++) {
        mx[i] = calloc(LARGURA_MX, sizeof(int));
        if (mx[i] == NULL) {
            printf("Erro: memoria insuficiente\n");
            exit(EXIT_FAILURE);
        }
    }
}

void Liberar_matriz_real() {
    if (!mx) return;

    for (int i = 0; i < ALTURA_MX; i++) {
        free(mx[i]);
    }
    free(mx);
}

void preencheMatriz() {
    srand(SEED);
    for (int i = 0; i < ALTURA_MX; i++) {
        for (int j = 0; j < LARGURA_MX; j++) {
            mx[i][j] = rand() % 32000;
        }
    }
}

int ehPrimo(int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;

    int lim = (int)sqrt(n);
    for (int i = 3; i <= lim; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}


void inicializador() {

    printf("Alocando e preenchendo a matriz, Aguarde...\n");
    Alocar_matriz_real();
    preencheMatriz();

    char choice;

    //mcGlobal = { 0,0 };

    do {
        printf("\nEscolha a busca a ser executada: \n\n");
        printf("1 - Executar Busca Sequencial\n");
        printf("2 - Executar Busca Paralela\n");
        printf("3 - Executar ambas as buscas\n");
        printf("Q - Sair\n");
        scanf(" %c", &choice);

        if (choice == '1') {
            buscaSequencial();
        }
        else if (choice == '2') {
            buscaParalela();
        }
        else if (choice == '3') {
            buscaSequencial();
            qtdPrimos = 0;
            buscaParalela();
        }

        qtdPrimos = 0;

    } while (choice != 'Q' && choice != 'q');

    Liberar_matriz_real();

}


// MAIN
int main() {

    inicializador();
    return 0;
}
