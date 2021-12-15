#include "common.h"

class Jit;

class InstructionBase{
    public:
        std::string name;
        InstructionBase(std::string name);
        virtual void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit) = 0;
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