#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datastructures.h"

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library


t_fragmnt read_and_fragment(char * path, int length /*em bits*/){
    int i, fsize=0, total_blocks=0;
    t_fragmnt ret;
    ret.block_amount = 0;
    ret.block_size = 0;
    ret.memory = NULL;
    FILE * fp;
    unsigned char ** buff = NULL;

    //transformo tamanho em bits para bytes (unidade minima)
    length = length/8 + ((length%8==0)? 0 : 1);
    
    if(!(fp = fopen(path, "r")))
        return ret;

    //descubro tamanho do arquivo em bytes
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    rewind(fp);// volta pro começo

    //calculo total de blocos sem os extras pra padding
    ret.block_amount = fsize/length;
    int resto = fsize%length;
    ret.block_size = length; //tamanho de cada bloco (em bytes)
    
    buff = (unsigned char**) malloc(ret.block_amount);

    for(i=0;i<ret.block_amount;i++){
        //aloco e zero a memoria para cada bloco
        buff[i] = (unsigned char *) malloc(length);
        memset(buff[i], '\0', length);
        //leio do arquivo e armazeno no bloco
        fread(buff[i], length, 1, fp);
    }

    //considero no total de blocos o extra, se existir, para padding
    ret.block_amount += (resto==0)? 0 : 1;

    for(int j=0;j<resto;j++){
        //insiro o excente no bloco de padding
        fread(&(buff[i][j]),1,1,fp);
    }
    
    //salvo a memoria alocada para ser retornada
    ret.memory=buff;

    //fecha o arquivo
    if(fp)
        fclose(fp);

    return ret;
}

void free_fragment(t_fragmnt f){
    if(f.block_amount != 0 && f.block_size !=0){
        for(int i=0; i<f.block_amount; i++){
            free(f.memory[i]);
        }
    }
}

int main(int argc, char ** argv)
{
    t_fragmnt x;
    int i;
    int tamanho_em_bits=32;
    int tamanho_em_bytes=tamanho_em_bits/8;
    //tamanho_em_bytes+= (tamanho_em_bits%32==0)? 0 : 1;
/*
    x = read_and_fragment("ArqTeste.txt",tamanho_em_bits);

    printf("x.tamanho = %d\n", x.tamanho);

    int total_de_blocos = x.tamanho/tamanho_em_bytes + ((x.tamanho%tamanho_em_bytes==0)?0:1);

    for(i=0;i<total_de_blocos;i++){
        printf("> %s\n", (x.memory)[i]);
    }
*/
    return 0;
}