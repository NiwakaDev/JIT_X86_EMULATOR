#include "common.h"

class Instruction;
#define  MEM_SIZE 1024 * 1024
#define INIT_EIP 0x7c00
#define INSTRUCTION_SIZE 256

enum REGISTER_KIND {EAX , ECX, EDX, EBX, ESP, EBP, ESI, EDI, REGISTER_KIND_TOTAL};

class Jit:public Object{
    public:
        Instruction* instructions[INSTRUCTION_SIZE];
        uint8_t* mem;
        uint32_t eip;
        uint32_t save_registers_[REGISTER_KIND_TOTAL];//保存領域
        Xbyak::CodeGenerator* eip2code[MEM_SIZE];     
        Jit();
        void Run();
        Xbyak::CodeGenerator* CompileBlock();
        bool IsCompiledBlock(uint32_t eip);
        union{
            uint32_t raw;
            struct{
                unsigned CF : 1;
                unsigned RESERVED0 : 1;
                unsigned PF : 1;
                unsigned RESERVED1 : 1;
                unsigned AF : 1;
                unsigned RESERVED2 : 1;
                unsigned ZF : 1;
                unsigned SF : 1;
                unsigned TF : 1;
                unsigned IF : 1;
                unsigned DF : 1;
                unsigned OF : 1;
                unsigned IOPL : 2;
                unsigned NT : 1;
                unsigned RESERVED3 : 1;
                unsigned RF : 1;
                unsigned VM : 1;
                unsigned AC : 1;
                unsigned VIF : 1;
                unsigned VIP : 1;
                unsigned ID : 1;
            }flgs;
        }eflags;
        uint32_t Read32(uint32_t addr);
        void ShowRegisters();
};