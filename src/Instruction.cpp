#include "Instruction.h"
#include "Jit.h"

using namespace std;

InstructionBase::InstructionBase(string name){
    this->code_name = name;
}

inline void InstructionBase::ParseModRM(Jit* jit){
    uint8_t code;
    code = jit->mem[jit->eip];
    this->modrm.mod = ((code&0xC0)>>6);
    this->modrm.op_code = ((code&0x38) >> 3);
    this->modrm.rm = code & 0x07;
    jit->eip++;
    //SIB判定
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: modrm.mod=0x%02X, modrm.rm=0x%02X", this->modrm.mod, this->modrm.rm);
    }
    //disp取得disp32は
    if((this->modrm.mod==0 && this->modrm.rm==5) || this->modrm.mod==2){
        //this->modrm.disp32 = emu->mem->Read32(jit->GetLinearAddrForCodeAccess());
        this->modrm.disp32 = *(uint32_t*)(jit->mem+jit->eip);
        jit->eip += 4;
    }else if(this->modrm.mod==1){
        //this->modrm.disp8 = emu->mem->Read8(jit->GetLinearAddrForCodeAccess());
        this->modrm.disp8 = jit->mem[jit->eip];
        jit->eip += 1;
    }
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
            this->Error("Not implemented: register_type=%d at %s::CompileStep\n", register_type, this->code_name.c_str());
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

Code83::Code83(string name):InstructionBase(name){
    for(int i=0; i<INSTRUCTION_SET_SMALL_SIZE; i++){
        this->instructions[i] = NULL;
    }
    this->instructions[5] = new SubRm32Imm8("SubRm32Imm8");
}

void Code83::CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit){
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

    jit->eip++;
    this->ParseModRM(jit);
    if(this->instructions[this->modrm.reg_index]==NULL){
            this->Error("code 83 /%02X is not implemented %s::Run", this->modrm.reg_index, this->code_name.c_str());
    }
    this->instructions[this->modrm.reg_index]->SetModRM(&this->modrm);
    this->instructions[this->modrm.reg_index]->CompileStep(code, stop, jit);
    return;
}

SubRm32Imm8::SubRm32Imm8(string name):InstructionBase(name){

}

void SubRm32Imm8::CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit){
    using namespace Xbyak::util;
	using namespace Xbyak;
	const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
	const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
	const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
	const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
	const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
	const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
	const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
	const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    *stop = true;
    uint32_t imm8 = (int32_t)(int8_t)jit->mem[jit->eip];
    jit->eip++;
    uint32_t rm32;
    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;

    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());
        if(this->modrm.rm==5){
            addr = this->modrm.disp32;
            addr = jit->GetLinearAddrForDataAccess(addr);
        }
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        addr = jit->save_registers_[(REGISTER_KIND)this->modrm.rm];
        addr = jit->GetLinearAddrForDataAccess(addr);
    }else if(this->modrm.mod==1){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        disp8 = (int32_t)this->modrm.disp8;
        addr = jit->save_registers_[(REGISTER_KIND)this->modrm.rm]+disp8;
        addr = jit->GetLinearAddrForDataAccess(addr);
    }else if(this->modrm.mod==2){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        disp32 = (int32_t)this->modrm.disp32;
        addr = jit->save_registers_[(REGISTER_KIND)this->modrm.rm]+disp32;
        addr = jit->GetLinearAddrForDataAccess(addr);
    }else if(this->modrm.mod==3){
        REGISTER_KIND register_kind = (REGISTER_KIND)this->modrm.rm;
        switch(register_kind){
            case ESP:
                code->sub(jit_esp, imm8);
                break;
            default:
                this->Error("Not implemented: register_kind=%d at %s::Run\n", register_kind, this->code_name.c_str());
        }
    }
    //TODO:
    //余計なフラグ情報まで更新している。
    //特定のフラグのみを更新するようにすべき。
    //フラグ更新処理
    code->pushfq();
    code->pop(jit_eflags);
    return;
}

MovRm32R32::MovRm32R32(string name):InstructionBase(name){

}

void MovRm32R32::CompileStep(Xbyak::CodeGenerator* code, bool* stop, Jit* jit){
    using namespace Xbyak::util;
	using namespace Xbyak;
	const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
	const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
	const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
	const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
	const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
	const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
	const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
	const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    *stop = true;
    uint32_t imm8 = (int32_t)(int8_t)jit->mem[jit->eip];
    jit->eip++;
    uint32_t rm32;
    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;

    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());
        if(this->modrm.rm==5){
            addr = this->modrm.disp32;
            addr = jit->GetLinearAddrForDataAccess(addr);
        }
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        addr = jit->save_registers_[(REGISTER_KIND)this->modrm.rm];
        addr = jit->GetLinearAddrForDataAccess(addr);
    }else if(this->modrm.mod==1){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        disp8 = (int32_t)this->modrm.disp8;
        addr = jit->save_registers_[(REGISTER_KIND)this->modrm.rm]+disp8;
        addr = jit->GetLinearAddrForDataAccess(addr);
    }else if(this->modrm.mod==2){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        disp32 = (int32_t)this->modrm.disp32;
        addr = jit->save_registers_[(REGISTER_KIND)this->modrm.rm]+disp32;
        addr = jit->GetLinearAddrForDataAccess(addr);
    }else if(this->modrm.mod==3){
        REGISTER_KIND register_kind = (REGISTER_KIND)this->modrm.rm;
        switch(register_kind){
            case ESP:
                code->sub(jit_esp, imm8);
                break;
            default:
                this->Error("Not implemented: register_kind=%d at %s::Run\n", register_kind, this->code_name.c_str());
        }
    }
    //TODO:
    //余計なフラグ情報まで更新している。
    //特定のフラグのみを更新するようにすべき。
    //フラグ更新処理
    code->pushfq();
    code->pop(jit_eflags);
    return;
}