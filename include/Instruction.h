#include "common.h"
#include "ModRM.h"

class Jit;
#define INSTRUCTION_SET_SMALL_SIZE 8

class InstructionBase:public Object{
    protected:
        ModRM modrm;
        void ParseModRM(Jit* jit);
    public:
        std::string code_name;
        InstructionBase(std::string name);
        virtual void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit) = 0;
        void SetModRM(ModRM* modrm){
            this->modrm = *modrm;
        }
};

class MovR32Imm32:public InstructionBase{
    public:
        MovR32Imm32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class JmpRel8:public InstructionBase{
    public:
        JmpRel8(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class Code83:public InstructionBase{
    private:
        InstructionBase* instructions[INSTRUCTION_SET_SMALL_SIZE];
    public: 
        Code83(std::string code_name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class SubRm32Imm8:public InstructionBase{
    public:
        SubRm32Imm8(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class MovRm32R32:public InstructionBase{
    public:
        MovRm32R32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};