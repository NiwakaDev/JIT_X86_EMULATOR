#include "common.h"
#include "Jit.h"

int main(int argc, char** argv){
    if(argc!=2){
        fprintf(stderr, "実行ファイルを選択してくれ");
        return 1;
    }
    Jit* jit = new Jit();
    FILE* binary = fopen(argv[1], "rb");
    if(binary==NULL){
        fprintf(stderr, "ファイルが開けない");
        return 1;
    }
    jit->eip = 0x7c00;
    fread(jit->mem + 0x7c00, 1, 0x200, binary);
    fprintf(stderr, "before:\n");
    jit->ShowRegisters();
    while(jit->eip<MEM_SIZE){
        jit->Run();
        if(jit->eip==0){
            fprintf(stderr, "after:\n");
            jit->ShowRegisters();
            fprintf(stderr, "End of program\n");
            break;
        }
    }   
}