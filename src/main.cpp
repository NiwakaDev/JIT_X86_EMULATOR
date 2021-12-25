#include "common.h"
#include "Jit.h"

int main(int argc, char** argv){
    #ifdef DEBUG
        FILE* out = NULL;
        out = fopen("niwaka_output.txt", "w");
        if(out==NULL){
            fprintf(stderr, "Error: fopen");
            exit(1);
        }
        //fprintf(stderr, "DEBUG\n");
    #else
        //fprintf(stderr, "NO-DEBUG\n");
    #endif

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
    //fprintf(stderr, "before:\n");
    //jit->ShowRegisters();
    int idx=1;
    while(jit->eip<MEM_SIZE){
        #ifdef DEBUG
            fprintf(out, "i:%d\n", idx);
            jit->Debug(out);
            jit->Run();
            jit->Debug(out);
            idx++;
        #else
            jit->Run();
        #endif
        //fprintf(stderr, "eip=%08X\n", jit->eip);
        if(jit->eip==0){
        //if(jit->save_registers_[EAX]==37){
            //fprintf(stderr, "after:\n");
            //jit->ShowRegisters();
            //fprintf(stderr, "End of program\n");
            break;
        }
    }   
    #ifdef DEBUG
        fclose(out);
    #endif
}