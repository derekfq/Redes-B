#include <stdio.h>
#include <stdlib.h>

typedef struct pacote_UDP{
    unsigned short int  porta_Origem,
                        porta_Destino,
                        Tamanho,
                        Checksum; //2 bytes %hu
    unsigned int        Dados; //2 or 4 bytes 	%u

}pacote_UDP;

pacote_UDP * cria_Pacote (){
    pacote_UDP * f = (pacote_UDP *) malloc ( sizeof (pacote_UDP));    // alocando espaço da estrutura
    return f;
}

void consulta_Pacote(pacote_UDP* P){
    printf("\nPorta de Origem:%hu",P->porta_Origem);
    printf("\nPorta de Destino:%hu",P->porta_Destino);
    printf("\nTamanho:%hu",P->Tamanho);
    printf("\nChecksum:%hu",P->Checksum);
    printf("\nDados:%u",P->Dados);
}

void preenche_Pacote(pacote_UDP* P){
    printf("\nPorta de Origem:");
    scanf("%hu",&P->porta_Origem);
    printf("\nPorta de Destino:");
    scanf("%hu",&P->porta_Destino);
    printf("\nChecksum:");
    printf("\nDados:");
    scanf("%u",&P->Dados);
    P->Tamanho=sizeof(P->porta_Origem) + sizeof(P->porta_Destino) + sizeof(P->Dados) + sizeof(P->Tamanho) + sizeof(P->Checksum);
}
// libera memoria (free)
void libera_Pacote(pacote_UDP* P){
    free(P);
}

int main()
{
pacote_UDP *P = cria_Pacote();
preenche_Pacote(P);
consulta_Pacote(P);


    return 0;
}
