#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tst.h"

int main(int argc,char** argv) {
    tst<char*>* linetst=new tst<char*>(2,NULL);
    tst<size_t>* lengthtst=new tst<size_t>(2,0);
    char* line;
    FILE* input;
    
    printf("Taille d'un noeud char* : %i\n",sizeof(tst_node<char*>));
    printf("Taille d'un noeud size_t : %i\n",sizeof(tst_node<int>));
    
    input = fopen("prenoms.txt","r");
    line = (char*)malloc(256);
    while(fgets(line,256,input)!=NULL) {
        linetst->put(line,line);
        lengthtst->put(line,strlen(line));
        line = (char*)malloc(256);
    }
    fclose(input);

    input = fopen("prenoms.txt","r");
    line = (char*)malloc(256);
    while(fgets(line,256,input)!=NULL) {
        printf("%s ==> %s (%i)",line,linetst->get(line),lengthtst->get(line));
        free(line);
        line = (char*)malloc(256);
    }
    fclose(input);
    free(line);

    linetst->adjust();
    lengthtst->adjust();
    printf("Taille totale line : %i\n",linetst->bytes_allocated());
    printf("Taille totale length : %i\n",lengthtst->bytes_allocated());

    delete linetst;
    delete lengthtst;
    return 0;
}