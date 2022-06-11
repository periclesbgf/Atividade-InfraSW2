#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 256
#define MIN 128
#define TLBN 16

int lookIntoPageTable();
void adicionarValorNegativoATabeladepaginas();
void adicionarValorNaTabelaDePaginas();
void copiar_para_memoria_fisica();
int buscarNaMemoriaFisica();
void printaTela();
void atualizarPageTable();
int procurarPeloMenorTempo();
int lookIntoPageTLB_FIFO();
int lookIntoPageTLB_LRU();
void adicionarValorNegativoATLB();
void adicionarValorNaTLB_FIFO();
void implementacaoFIFO();
void implementacaoLRU();
int menorTempoTLB();
void adicionarValorNegativoATLB_LRU();
void atualizarPageTable_FIFO();
void copiar_para_memoria_fisica_FIFO();
int adicionarValorNaTabelaDePaginas_FIFO();
void adicionarValorNegativoATabeladepaginas_FIFO();
int lookIntoPageTable_FIFO();

typedef struct pageTable
{
        int frame;
        int bit;
        int tempo;
}pageTable;

typedef struct memoriaFisica
{
        char bin[MAX];
}memoriaFisica;

typedef struct tlbs
{
        int frame;
        int pageNumber;
        int bit;
}tlbs;

typedef struct tlbLru
{
        int frame;
        int pageNumber;
        int bit;
        int tempo_tlb;
}tlbLru;

memoriaFisica memoria_fisica[128];
pageTable page_table[MAX];
tlbs tlb_FIFO[TLBN];
tlbLru tlb_LRU[TLBN];

int countFila = 0;
int page_foult = 0;
int isMemCheia = 0;
int primeiro = 1;
int tempoGeral = 0;
int tlb_hit = 0;
int countTLB_FIFO = 0;
int tempoGeralTlb = 0;

int main (int argc, char *argv[]){
        FILE *file;
        FILE *backingStore;
        FILE *prints;

        char frase[1000];
        int argumento_memoria = 0;
        int argumento_tlb = 0;

        if (strcmp("addresses.txt", argv[1]) == 0){
                file = fopen(argv[1], "r");
        } else {
                printf("Digite o nome do arquivo correto: addresses.txt");
                exit (0);
        }

        prints = fopen("prints.txt", "w");

        if (file == NULL){
                printf("Erro ao acessar o arquivo.");
                fclose(file);
                exit (0);
        }

        if (strcmp("fifo", argv[3]) == 0){
                argumento_tlb = 1;
        } else if (strcmp("lru", argv[3]) == 0){
                argumento_tlb = 2;

        } else {
                printf("Digite um argumento valido");
                fclose(file);
                fclose(prints);
                exit (0);
        }

        if (strcmp("fifo", argv[2]) == 0){
                argumento_memoria = 1;
        } else if (strcmp("lru", argv[2]) == 0){
                argumento_memoria = 2;
        } else {
                printf("Digite um argumento valido");
                fclose(file);
                fclose(prints);
                exit (0);
        }
        adicionarValorNegativoATabeladepaginas_FIFO();
        adicionarValorNegativoATabeladepaginas();
        adicionarValorNegativoATLB();
        adicionarValorNegativoATLB_LRU();
        if (argumento_memoria == 1){
                implementacaoFIFO(backingStore, prints, file, argumento_tlb, frase);
        } else if (argumento_memoria == 2){
                implementacaoLRU(backingStore, prints, file, argumento_tlb, frase);
        } else {
                printf("Digite um argumento valido para implementacao na memoria");
                fclose(file);
                fclose(prints);
                exit(0);
        }
        fprintf(prints, "Number of Translated Addresses = %d\n", tempoGeral);
        fprintf(prints, "Page Faults = %d\n", page_foult);
        float page_foult_Rate = (float) page_foult/1000;
        fprintf(prints, "Page Fault Rate = %.3f\n", page_foult_Rate);
        fprintf(prints, "TLB Hits = %d\n", tlb_hit);
        fclose(file);
        fclose(prints);

        return 0;
}

int lookIntoPageTable(int pageNumber){
        if (page_table[pageNumber].bit == -1){
                page_foult++;
                return -1;
        } else {
                return page_table[pageNumber].frame;
        }
}

void adicionarValorNegativoATabeladepaginas(){
        for (int i = 0; i < 256; i++)
        {
                page_table[i].bit = -1;
                page_table[i].frame = -1;
                page_table[i].tempo = -1;
        }
}

void adicionarValorNaTabelaDePaginas(int page_number, int frame){
        page_table[page_number].frame = frame;
        page_table[page_number].bit = 1;
        page_table[page_number].tempo = tempoGeral;
}

void copiar_para_memoria_fisica(FILE* backingStore, int pageNumber, int index){
        backingStore = fopen("BACKING_STORE.bin", "rb");
        fseek(backingStore, 256 * pageNumber, SEEK_SET);
        fread(memoria_fisica[index].bin, 256, 1, backingStore);
        fclose(backingStore);
        return;
}

int buscarNaMemoriaFisica(int frame, int offset){
        return memoria_fisica[frame].bin[offset];
}

void printaTela(int n, int fisico, int valor, FILE *prints){
        fprintf(prints, "Virtual address: %d Physical address: %u Value: %d\n", n, fisico, valor);
}

void atualizarPageTable (int index){
        for (int i = 0; i < 256; i++)
        {
                if (page_table[i].frame == index){
                        page_table[i].bit = -1;
                        page_table[i].frame = -1;
                        page_table[i].tempo = -1;
                }
        }
}

int procurarPeloMenorTempo(){
        int posicao;
        int menor = 5000;
        int i;
        for (i = 0; i < 256; i++)
        {
                if (page_table[i].tempo < menor && page_table[i].tempo != -1 && page_table[i].bit == 1){
                        menor = page_table[i].tempo;
                        posicao = page_table[i].frame;
                }
        }
        return posicao;
}

void adicionarValorNegativoATLB(){
        for (int i = 0; i < TLBN; i++)
        {
                tlb_FIFO[i].bit = -1;
                tlb_FIFO[i].frame = -1;
                tlb_FIFO[i].pageNumber = -1;
        }
}

void adicionarValorNegativoATLB_LRU(){
        for (int i = 0; i < TLBN; i++)
        {
                tlb_LRU[i].bit = -1;
                tlb_LRU[i].frame = -1;
                tlb_LRU[i].pageNumber = -1;
                tlb_LRU[i].tempo_tlb = -1;
        }
}

int lookIntoPageTLB_FIFO(int pageNumber) {
        for (int i = 0; i < TLBN; i++)
        {
                if (tlb_FIFO[i].pageNumber == pageNumber && tlb_FIFO[i].bit == 1) {
                        return tlb_FIFO[i].frame;
                }
                else if (i == 15 && tlb_FIFO[i].pageNumber != pageNumber)
                {
                        return -1;
                }
        }
}

int lookIntoPageTLB_LRU(int pageNumber) {
        for (int i = 0; i < TLBN; i++)
        {
                if (tlb_LRU[i].pageNumber == pageNumber) {
                        if (tlb_LRU[i].frame < 0){
                                return -1;
                        } else {
                                tlb_LRU[i].tempo_tlb = tempoGeralTlb;
                                return tlb_LRU[i].frame;
                        }
                }
        }
        return -1;
}

void adicionarValorNaTLB_FIFO(int frame, int pageNumber){
        if (countTLB_FIFO < 16) {
                tlb_FIFO[countTLB_FIFO].frame = frame;
                tlb_FIFO[countTLB_FIFO].bit = 1;
                tlb_FIFO[countTLB_FIFO].pageNumber = pageNumber;
                countTLB_FIFO++;
        }
        if (countTLB_FIFO == 16) {
                countTLB_FIFO = 0;
                return;
        }
}

void adicionarValorNaTLB_LRU(int frame, int pageNumber){
        if (countTLB_FIFO < 16) {
                tlb_LRU[countTLB_FIFO].frame = frame;
                tlb_LRU[countTLB_FIFO].bit = 1;
                tlb_LRU[countTLB_FIFO].pageNumber = pageNumber;
                tlb_LRU[countTLB_FIFO].tempo_tlb = tempoGeralTlb;
                tempoGeralTlb++;
                countTLB_FIFO++;
                return;
        } else {
                int posix = menorTempoTLB();
                tlb_LRU[posix].frame = frame;
                tlb_LRU[posix].bit = 1;
                tlb_LRU[posix].tempo_tlb = tempoGeralTlb;
                tlb_LRU[posix].pageNumber = pageNumber;
                tempoGeralTlb++;
                return;
        }
}

int menorTempoTLB () {
        int posicao;
        int menor = 5000;
        int i;
        for (i = 0; i < 16; i++)
        {
                if (tlb_LRU[i].tempo_tlb < menor){
                        menor = tlb_LRU[i].tempo_tlb;
                        posicao = i;
                }
        }
        return posicao;
}

int lookIntoPageTable_FIFO(int pageNumber){
        if (page_table[pageNumber].bit == -1){
                page_foult++;
                return -1;
        } else {
                return page_table[pageNumber].frame;
        }
}

void adicionarValorNegativoATabeladepaginas_FIFO(){
        for (int i = 0; i < 256; i++)
        {
                page_table[i].bit = -1;
                page_table[i].frame = -1;
        }
}

int adicionarValorNaTabelaDePaginas_FIFO(int page_number){
        page_table[page_number].frame = countFila;
        page_table[page_number].bit = 1;
        return countFila;
}

void copiar_para_memoria_fisica_FIFO(FILE* backingStore, int pageNumber){
        backingStore = fopen("BACKING_STORE.bin", "rb");
        fseek(backingStore, 256 * pageNumber, SEEK_SET);
        fread(memoria_fisica[countFila].bin, 256, 1, backingStore);
        fclose(backingStore);
        return;
}

void atualizarPageTable_FIFO(){
        for (int i = 0; i < 256; i++)
        {
                if (page_table[i].frame == countFila){
                        page_table[i].bit = -1;
                        page_table[i].frame = -1;
                }
        }
}

void implementacaoFIFO(FILE *backingStore, FILE *prints, FILE *file,  int argumento_tlb, char *frase){
        while (fgets(frase, 1000, file) != NULL) {
                int n = atoi(frase);
                int frame;
                int fisico;
                int offset = n & 255;
                int pageNumber = (n >> 8) & 255;
                int leitura;
                int valor;
                if (argumento_tlb == 1){
                        leitura = lookIntoPageTLB_FIFO(pageNumber);
                } else if (argumento_tlb == 2){
                        leitura = lookIntoPageTLB_LRU(pageNumber);
                }
                if (leitura == -1){
                        int is_In_PageTable = lookIntoPageTable_FIFO(pageNumber);
                        if (is_In_PageTable == -1){
                                if (isMemCheia == 128){
                                        if (primeiro == 1 || countFila == 127)
                                        {
                                                countFila = 0;
                                                primeiro = 1;
                                        }
                                        countFila = primeiro - 1;
                                        primeiro ++;
                                        atualizarPageTable_FIFO(countFila);
                                        copiar_para_memoria_fisica_FIFO(backingStore, pageNumber);
                                        adicionarValorNaTabelaDePaginas_FIFO(pageNumber);
                                        frame = lookIntoPageTable_FIFO(pageNumber);
                                        valor = buscarNaMemoriaFisica(frame, offset);
                                        fisico = frame*256 + offset;
                                } else {
                                        copiar_para_memoria_fisica_FIFO(backingStore, pageNumber);
                                        adicionarValorNaTabelaDePaginas_FIFO(pageNumber);
                                        countFila++;
                                        frame = lookIntoPageTable_FIFO(pageNumber);
                                        valor = buscarNaMemoriaFisica(frame, offset);
                                        fisico = frame*256 + offset;
                                        isMemCheia++;
                                }
                                if (argumento_tlb == 1){
                                        adicionarValorNaTLB_FIFO(frame, pageNumber);
                                } else if (argumento_tlb == 2){
                                        adicionarValorNaTLB_LRU(frame, pageNumber);
                                }
                                //leitura = lookIntoPageTLB_FIFO(pageNumber);

                        } else {
                                frame = lookIntoPageTable_FIFO(pageNumber);
                                fisico = 256*frame + offset;
                                valor = buscarNaMemoriaFisica(frame, offset);
                                if (argumento_tlb == 1){
                                        adicionarValorNaTLB_FIFO(frame, pageNumber);
                                } else if (argumento_tlb == 2){
                                        adicionarValorNaTLB_LRU(frame, pageNumber);
                                }
                        }
                } else {
                        tlb_hit++;
                        frame = lookIntoPageTable_FIFO(pageNumber);
                        fisico = 256*frame + offset;
                        valor = buscarNaMemoriaFisica(frame, offset);
                }
                printaTela(n, fisico, valor, prints);
                tempoGeral++;
        }

}

void implementacaoLRU(FILE *backingStore, FILE *prints, FILE *file,  int argumento_tlb, char *frase){
        //fpos_t position;
        char c[10];
        //fgetpos(file, &position);
        while (fscanf(file, "%s", &c) != EOF) {
                int n = atoi(c);
                int frame;
                int fisico;
                int offset = n & 255;
                int pageNumber = (n >> 8) & 255;
                int frameMemoriaDeMenorUso;
                int leitura;
                int valor;
                if (argumento_tlb == 1){
                        leitura = lookIntoPageTLB_FIFO(pageNumber);
                } else if (argumento_tlb == 2){
                        leitura = lookIntoPageTLB_LRU(pageNumber);
                } else {
                        printf("Digite um argumento valido para a tlb");
                }
                if (leitura == -1){
                        int is_In_PageTable = lookIntoPageTable(pageNumber);
                        if (is_In_PageTable == -1){
                                if (isMemCheia == MIN){
                                        frameMemoriaDeMenorUso = procurarPeloMenorTempo();
                                        atualizarPageTable(frameMemoriaDeMenorUso);
                                        copiar_para_memoria_fisica(backingStore, pageNumber, frameMemoriaDeMenorUso);
                                        adicionarValorNaTabelaDePaginas(pageNumber, frameMemoriaDeMenorUso);
                                        frame = frameMemoriaDeMenorUso;
                                        valor = memoria_fisica[frame].bin[offset];
                                        fisico = frame*256 + offset;

                                } else {
                                        frame = countFila;
                                        copiar_para_memoria_fisica(backingStore, pageNumber, frame);
                                        adicionarValorNaTabelaDePaginas(pageNumber, frame);
                                        countFila++;
                                        frame = lookIntoPageTable(pageNumber);
                                        valor = buscarNaMemoriaFisica(frame, offset);
                                        fisico = frame*256 + offset;
                                        page_table[pageNumber].tempo = tempoGeral;
                                        isMemCheia++;
                                }
                                if (argumento_tlb == 1){
                                        adicionarValorNaTLB_FIFO(frame, pageNumber);
                                } else if (argumento_tlb == 2){
                                        adicionarValorNaTLB_LRU(frame, pageNumber);
                                }
                                //fsetpos(file, &position);
                        }
                        if(is_In_PageTable != -1) {
                                frame = is_In_PageTable;
                                page_table[pageNumber].tempo = tempoGeral;
                                fisico = 256*frame + offset;
                                valor = buscarNaMemoriaFisica(frame, offset);
                                if (argumento_tlb == 1){
                                        adicionarValorNaTLB_FIFO(frame, pageNumber);
                                } else if (argumento_tlb == 2){
                                        adicionarValorNaTLB_LRU(frame, pageNumber);
                                }
                        }
                        printaTela(n, fisico, valor, prints);
                        tempoGeral++;
                } else {
                        //fgetpos(file, &position);
                        tlb_hit++;
                        page_table[pageNumber].tempo = tempoGeral;
                        fisico = 256*leitura + offset;
                        valor = buscarNaMemoriaFisica(leitura, offset);
                        printaTela(n, fisico, valor, prints);
                        tempoGeral++;
                }
        }
}
