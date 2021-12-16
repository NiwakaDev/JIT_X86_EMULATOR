#include "common.h"
#include "ModRM.h"

class Jit;
#define INSTRUCTION_SET_SMALL_SIZE 8

class Instruction:public Object{
    protected:
        ModRM modrm;
        void ParseModRM(Jit* jit);
        void Pop32(Xbyak::CodeGenerator* code, Jit* jit, const Xbyak::Address dest_addr, const Xbyak::Reg64 mem);
    public:
        std::string code_name;
        Instruction(std::string name);
        virtual void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit) = 0;
        void SetModRM(ModRM* modrm){
            this->modrm = *modrm;
        }
};

class MovR32Imm32:public Instruction{
    public:
        MovR32Imm32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class JmpRel8:public Instruction{
    public:
        JmpRel8(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class Code83:public Instruction{
    private:
        Instruction* instructions[INSTRUCTION_SET_SMALL_SIZE];
    public: 
        Code83(std::string code_name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class SubRm32Imm8:public Instruction{
    public:
        SubRm32Imm8(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class MovRm32R32:public Instruction{
    public:
        MovRm32R32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class MovRm32Imm32:public Instruction{
    public:
        MovRm32Imm32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class AddRm32R32:public Instruction{
    public:
        AddRm32R32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class MovR32Rm32:public Instruction{
    public:
        MovR32Rm32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class CodeFF:public Instruction{
    private:
        Instruction* instructions[INSTRUCTION_SET_SMALL_SIZE];
    public:
        CodeFF(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class IncRm32:public Instruction{
    public:
        IncRm32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class JmpRel32:public Instruction{
    public:
        JmpRel32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class CallRel32:public Instruction{
    public:
        CallRel32(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};

class Ret32Near:public Instruction{
    public:
        Ret32Near(std::string name);
        void CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit);
};