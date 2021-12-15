#include "common.h"

class InstructionBase;
#define  MEM_SIZE 1024 * 1024
#define INIT_EIP 0x7c00
#define INSTRUCTION_SIZE 256

enum REGISTER_KIND {EAX, EBX, ECX, EDX, EDI, ESI, EBP, ESP, REGISTER_KIND_TOTAL};

class Jit{
    public:
        InstructionBase* instructions[INSTRUCTION_SIZE];
        uint8_t* mem;
        uint32_t eip;
        uint8_t save_registers_[REGISTER_KIND_TOTAL];//保存領域
        Xbyak::CodeGenerator* eip2code[MEM_SIZE];     
        Jit();
        void Run();
        Xbyak::CodeGenerator* CompileBlock();
        bool IsCompiledBlock(uint32_t eip);
};