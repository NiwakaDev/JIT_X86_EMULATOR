#include "Instruction.h"
#include "Jit.h"

using namespace std;

InstructionBase::InstructionBase(string name){
    this->name = name;
}

MovR32Imm32::MovR32Imm32(string name):InstructionBase(name){

}

void MovR32Imm32::CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit){
    using namespace Xbyak::util;
	using namespace Xbyak;
	const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
	const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
	const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
	const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
	const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
	const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
	const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
	const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    uint8_t register_type = jit->mem[jit->eip]-0xB8;
    jit->eip++;
    uint32_t imm32 = jit->mem[jit->eip];
    jit->eip += 4;
    switch(register_type){
        case 0:
            code->mov(jit_eax, imm32);
            break;
        default:
            fprintf(stderr, "Not implemented: register_type=%d at %s::CompileStep\n", register_type, this->name.c_str());
            exit(1);
    }
    return;
}

JmpRel8::JmpRel8(string name):InstructionBase(name){

}

void JmpRel8::CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit){
    using namespace Xbyak::util;
	using namespace Xbyak;
	const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
	const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
	const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
	const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
	const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
	const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
	const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
	const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。

    *stop = true;//jmp命令では次にどこに飛べば良いかわからず、制御を本体に戻す。
    jit->eip++;
    int32_t rel8 = (int32_t)(int8_t)jit->mem[jit->eip];
    jit->eip = jit->eip+1+rel8;
    return;
}