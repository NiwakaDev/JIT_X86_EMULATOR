#include "Instruction.h"
#include "Jit.h"

using namespace std;
using namespace Xbyak::util;
using namespace Xbyak;


//呼び出し規約：
//https://motojiroxx.hatenablog.com/entry/2018/09/04/005142

void bios_error(uint64_t selector){
    fprintf(stderr, "Not implemented: selector=0x%02X at bios_error\n", selector);
    exit(1);
}

void put(uint8_t data){
    putchar(data);
}

static uint8_t in8(uint64_t port){
    switch(port){
        case 0x03F8:
            return getchar();
        default:
            fprintf(stderr, "Not implemented: port=%08X at in8\n", port);
            exit(1);
    }
}

static void out8(uint64_t port, uint8_t data){
    //fprintf(stderr, "port=%04X, data=%02X\n", port, data);
    switch(port){
        case 0x03F8:
            putchar(data);
            break;
        default:
            fprintf(stderr, "Not implemented: port=%08X at out8\n", port);
            exit(1);
    }
}

Instruction::Instruction(string name){
    this->code_name = name;
}

inline void Instruction::ParseModRM(Jit* jit, CodeGenerator* code){
    uint8_t op_code;
    op_code = jit->mem[jit->eip];
    this->modrm.mod = ((op_code&0xC0)>>6);
    this->modrm.op_code = ((op_code&0x38) >> 3);
    this->modrm.rm = op_code & 0x07;
    const Reg64 jit_eip(rbx);//jit_eipとして扱う。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip++;
    #else 
        jit->eip++;
    #endif
    //SIB判定
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: modrm.mod=0x%02X, modrm.rm=0x%02X", this->modrm.mod, this->modrm.rm);
    }
    //disp取得disp32は
    if((this->modrm.mod==0 && this->modrm.rm==5) || this->modrm.mod==2){
        //this->modrm.disp32 = emu->mem->Read32(jit->GetLinearAddrForCodeAccess());
        this->modrm.disp32 = *(uint32_t*)(jit->mem+jit->eip);
    #ifdef DEBUG
        code->add(dword [jit_eip], 4);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 4;
    #else 
        jit->eip += 4;
    #endif
    }else if(this->modrm.mod==1){
        //this->modrm.disp8 = emu->mem->Read8(jit->GetLinearAddrForCodeAccess());
        this->modrm.disp8 = jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    }
}

Reg32 Instruction::GetReg32ForRegIdx(){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    Reg32 r32;       
    switch((REGISTER_KIND)this->modrm.reg_index){
        case EAX:
            return jit_eax;
        case EBX:
            return jit_ebx;
        case ECX:
            return jit_ecx;
        case EDX:
            return jit_edx;
        case EDI:
            return jit_edi;
        case ESI:
            return jit_esi;
        case EBP:
            return jit_ebp;
        case ESP:
            return jit_esp;
        default:
            this->Error("Not implemented: (REGISTER_KIND)this->modrm.reg_index=%d at %s::GetReg32ForRegIdx", (REGISTER_KIND)this->modrm.reg_index, this->code_name.c_str());
    }
}

Reg32 Instruction::GetReg32(REGISTER_KIND register_kind){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    Reg32 r32;       
    switch(register_kind){
        case EAX:
            return jit_eax;
        case EBX:
            return jit_ebx;
        case ECX:
            return jit_ecx;
        case EDX:
            return jit_edx;
        case EDI:
            return jit_edi;
        case ESI:
            return jit_esi;
        case EBP:
            return jit_ebp;
        case ESP:
            return jit_esp;
        default:
            this->Error("Not implemented: (REGISTER_KIND)this->modrm.reg_index=%d at %s::GetReg32ForRegIdx", (REGISTER_KIND)this->modrm.reg_index, this->code_name.c_str());
    }
}

//mem       : jitのメモリ領域
//data      : pushされるデータ
void Instruction::Push32(CodeGenerator* code, Jit* jit, const Reg64 mem, uint32_t data){
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。

    code->mov(mem, (size_t)jit->mem);
    code->sub(jit_esp, 4);
    code->add(mem, jit_esp);
    code->mov(dword [mem], data);
}

//mem       : jitのメモリ領域
//reg       : pushされるレジスタ
void Instruction::Push32(CodeGenerator* code, Jit* jit, const Reg64 mem, const Reg32 reg){
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。

    code->mov(mem, (size_t)jit->mem);
    code->sub(jit_esp, 4);
    code->add(mem, jit_esp);
    code->mov(dword [mem], reg);
}

//dest_addr : 保存先アドレス
//mem       : jitのメモリ領域
void Instruction::Pop32(CodeGenerator* code, Jit* jit, const Address dest_addr, const Reg64 mem){
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 data(esi);

    code->add(mem, jit_esp);
    code->mov(esi, dword [mem]);
    code->mov(dest_addr, esi);  
    code->add(jit_esp, 4);
}

void Instruction::Pop32(CodeGenerator* code, Jit* jit, const Reg32 dest_reg, const Reg64 mem){
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 data(esi);

    code->add(mem, jit_esp);
    code->mov(dest_reg, dword [mem]);  
    code->add(jit_esp, 4);
}

MovR32Imm32::MovR32Imm32(string name):Instruction(name){

}

void MovR32Imm32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg64 jit_eip(rbx);//jit_eipとして扱う。
    Reg32 r32;
    REGISTER_KIND register_type = (REGISTER_KIND)(jit->mem[jit->eip]-0xB8);
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    uint32_t imm32 = *(uint32_t*)(jit->mem+jit->eip);
    #ifdef DEBUG
        code->add(dword [jit_eip], 4);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 4;
    #else 
        jit->eip += 4;
    #endif
    r32 = this->GetReg32(register_type);   
    code->mov(r32, imm32);
    return;
}

JmpRel8::JmpRel8(string name):Instruction(name){

}

void JmpRel8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg64 jit_eip(rbx);//jit_eipとして扱う。
    *stop = true;//jmp命令では次にどこに飛べば良いかわからず、制御を本体に戻す。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    int32_t rel8 = (int32_t)(int8_t)jit->mem[jit->eip];
    #ifdef DEBUG
        code->mov(dword [jit_eip], jit->eip+1+rel8);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip = jit->eip+1+rel8;
    #else 
        jit->eip = jit->eip+1+rel8;
    #endif
    return;
}

Code83::Code83(string name):Instruction(name){
    for(int i=0; i<INSTRUCTION_SET_SMALL_SIZE; i++){
        this->instructions[i] = NULL;
    }
    this->instructions[0] = new AddRm32Imm8("AddRm32Imm8");
    this->instructions[5] = new SubRm32Imm8("SubRm32Imm8");
}

void Code83::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
	const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
	const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
	const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
	const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
	const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
	const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
	const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
	const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg64 jit_eip(rbx);//jit_eipとして扱う。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);
        jit->eip++;
    #else
        jit->eip++;
    #endif
    this->ParseModRM(jit, code);
    if(this->instructions[this->modrm.reg_index]==NULL){
            this->Error("code 83 /%02X is not implemented %s::Run", this->modrm.reg_index, this->code_name.c_str());
    }
    this->instructions[this->modrm.reg_index]->SetModRM(&this->modrm);
    this->instructions[this->modrm.reg_index]->CompileStep(code, stop, jit);
    return;
}

SubRm32Imm8::SubRm32Imm8(string name):Instruction(name){

}

void SubRm32Imm8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg64 jit_eip(rbx);//jit_eipとして扱う。
    uint32_t imm8 = (int32_t)(int8_t)jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
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

    }else if(this->modrm.mod==1){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());

    }else if(this->modrm.mod==2){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());

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

MovRm32R32::MovRm32R32(string name):Instruction(name){

}

void MovRm32R32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg64 r32(rcx);       
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    this->ParseModRM(jit, code);

    uint32_t rm32;
    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;
    
    switch((REGISTER_KIND)this->modrm.reg_index){
        case EAX:
            code->mov(r32, jit_eax);
            break;
        case ESP:
            code->mov(r32, jit_esp);
            break;
        default:
            this->Error("Not implemented: (REGISTER_KIND)this->modrm.reg_index=%d at %s::CompileStep", (REGISTER_KIND)this->modrm.reg_index, this->code_name.c_str());
    }
    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());
    }else if(this->modrm.mod==1){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());

    }else if(this->modrm.mod==2){
        this->Error("Not implemented: %s::GetRM32", this->code_name.c_str());
    }else if(this->modrm.mod==3){
        REGISTER_KIND register_kind = (REGISTER_KIND)this->modrm.rm;
        switch(register_kind){
            case ECX:
                code->mov(jit_ecx, r32);
                break;
            case EBP:
                code->mov(jit_ebp, r32);
                break;
            default:
                this->Error("Not implemented: register_kind=%d at %s::Run\n", register_kind, this->code_name.c_str());
        }
    }
    return;
}

MovRm32Imm32::MovRm32Imm32(string name):Instruction(name){

}

void MovRm32Imm32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg64 r32(rcx);       
    const Reg32 effective_addr(edi);     
    const Reg64 mem(rdx);
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    this->ParseModRM(jit, code);
    uint32_t imm32 = jit->Read32(jit->eip);
    #ifdef DEBUG
        code->add(dword [jit_eip], 4);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 4;
    #else 
        jit->eip += 4;
    #endif

    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;
    code->mov(mem, (size_t)jit->mem);
    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());
    }else if(this->modrm.mod==1){
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        disp8 = (int32_t)this->modrm.disp8;
        switch ((REGISTER_KIND)this->modrm.rm){
            case EBP:
                code->mov(effective_addr, disp8);
                code->add(effective_addr, jit_ebp);
                break;
            default:
                this->Error("Not implemented: register_type=%d at %s::CompileStep\n", this->modrm.rm, this->code_name.c_str());
                break;
        }
        //jit->mem[effective_addr]++したい。
        code->add(mem, effective_addr);
        code->mov(dword [mem], imm32);
    }else if(this->modrm.mod==2){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());
    }else if(this->modrm.mod==3){
        REGISTER_KIND register_kind = (REGISTER_KIND)this->modrm.rm;
        switch(register_kind){
            case EAX:
                code->mov(jit_eax, imm32);
                break;
            default:
                this->Error("Not implemented: register_kind=%d at %s::Run\n", register_kind, this->code_name.c_str());
        }
        return;
    }
}

AddRm32R32::AddRm32R32(string name):Instruction(name){

}

void AddRm32R32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg64 r32(rcx);       
    const Reg32 effective_addr(edi);     
    const Reg64 mem(rdx);
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    this->ParseModRM(jit, code);
    code->mov(mem, (size_t)jit->mem);
    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;
    switch((REGISTER_KIND)this->modrm.reg_index){
        case EAX:
            code->mov(r32, jit_eax);
            break;
        case EBX:
            code->mov(r32, jit_ebx);
            break;
        case EDX:
            code->mov(r32, jit_edx);
            break;
        default:
            this->Error("Not implemented: (REGISTER_KIND)this->modrm.reg_index=%d at %s::Run", (REGISTER_KIND)this->modrm.reg_index, this->code_name.c_str());
    }
    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());
    }else if(this->modrm.mod==1){
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        disp8 = (int32_t)this->modrm.disp8;
        switch ((REGISTER_KIND)this->modrm.rm){
            case EBP:
                code->mov(effective_addr, disp8);
                code->add(effective_addr, jit_ebp);
                break;
            default:
                this->Error("Not implemented: register_type=%d at %s::CompileStep\n", this->modrm.rm, this->code_name.c_str());
                break;
        }
        //jit->mem[effective_addr]++したい。
        code->add(mem, effective_addr);
        code->add(dword [mem], r32);
    }else if(this->modrm.mod==2){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());
    }else if(this->modrm.mod==3){
        REGISTER_KIND register_kind = (REGISTER_KIND)this->modrm.rm;
        switch(register_kind){
            case EAX:
                code->add(jit_eax, r32);
                break;
            case ECX:
                code->add(jit_ecx, r32);
                break;
            default:
                this->Error("Not implemented: register_kind=%d at %s::CompileStep\n", register_kind, this->code_name.c_str());
        }
        return;
    }
    //TODO:
    //余計なフラグ情報まで更新している。
    //特定のフラグのみを更新するようにすべき。
    //フラグ更新処理
    code->pushfq();
    code->pop(jit_eflags);
    return;
}

MovR32Rm32::MovR32Rm32(string name):Instruction(name){

}

void MovR32Rm32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg32 effective_addr(edi);     
    const Reg64 mem(rdx);
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    this->ParseModRM(jit, code);

    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;
    code->mov(mem, (size_t)jit->mem);
    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());

    }else if(this->modrm.mod==1){
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        disp8 = (int32_t)this->modrm.disp8;
        switch ((REGISTER_KIND)this->modrm.rm){
            case EBP:
                code->mov(effective_addr, disp8);
                code->add(effective_addr, jit_ebp);
                break;
            default:
                this->Error("Not implemented: register_type=%d at %s::CompileStep\n", this->modrm.rm, this->code_name.c_str());
                break;
        }
        //jit->mem[effective_addr]++したい。
        code->add(mem, effective_addr);
        switch((REGISTER_KIND)this->modrm.reg_index){
            case EAX:
                code->mov(jit_eax, dword [mem]);
                break;
            case EDX:
                code->mov(jit_edx, dword [mem]);
                break;
            case ESI:
                code->mov(jit_esi, dword [mem]);
                break;
            case EDI:
                code->mov(jit_edi, dword [mem]);
                break;
            default:
                this->Error("Not implemented: (REGISTER_KIND)this->modrm.reg_index=%d at %s::CompileStep", (REGISTER_KIND)this->modrm.reg_index, this->code_name.c_str());
        }
        return;
    }else if(this->modrm.mod==2){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());
    }else if(this->modrm.mod==3){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());
    }
}

CodeFF::CodeFF(string code_name):Instruction(code_name){
    for(int i=0; i<INSTRUCTION_SET_SMALL_SIZE; i++){
        this->instructions[i] = NULL;
    }
    this->instructions[0] = new IncRm32("IncRm32");
}

void CodeFF::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    this->ParseModRM(jit, code);
    if(this->instructions[this->modrm.reg_index]==NULL){
            this->Error("Not implemented: FF /%02X at %s::Run", this->modrm.reg_index, this->code_name.c_str());
    }
    this->instructions[this->modrm.reg_index]->SetModRM(&this->modrm);
    this->instructions[this->modrm.reg_index]->CompileStep(code, stop, jit);
    return;
}

IncRm32::IncRm32(string name):Instruction(name){

}

void IncRm32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg64 rm32(rcx);       
    const Reg32 effective_addr(edi); // effective_addr
    const Reg64 mem(rdx);//jit->mem
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;

    code->mov(mem, (size_t)jit->mem);
    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());
    }else if(this->modrm.mod==1){
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::GetRM32", this->code_name.c_str());
        }
        disp8 = (int32_t)this->modrm.disp8;
        switch ((REGISTER_KIND)this->modrm.rm){
            case EBP:
                code->mov(effective_addr, disp8);
                code->add(effective_addr, jit_ebp);
                break;
            default:
                this->Error("Not implemented: register_type=%d at %s::CompileStep\n", this->modrm.rm, this->code_name.c_str());
                break;
        }
        //jit->mem[effective_addr]++したい。
        code->add(mem, effective_addr);
        code->inc(dword [mem]);
    }else if(this->modrm.mod==2){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());

    }else if(this->modrm.mod==3){
        this->Error("Not implemented: this->modrm.mod=%d at %s::GetRM32", this->modrm.mod, this->code_name.c_str());

    }

    //TODO:
    //余計なフラグ情報まで更新している。
    //特定のフラグのみを更新するようにすべき。
    //フラグ更新処理
    code->pushfq();
    code->pop(jit_eflags);
    return;
}

JmpRel32::JmpRel32(string name):Instruction(name){

}

void JmpRel32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    *stop = true;//jmp命令では次にどこに飛べば良いかわからず、制御を本体に戻す。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    uint32_t rel32 = jit->Read32(jit->eip);
    #ifdef DEBUG
        code->mov(dword [jit_eip], jit->eip + rel32 + 4);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip = jit->eip + rel32 + 4;
    #else 
        jit->eip = jit->eip + rel32 + 4;
    #endif
    return;
}

CallRel32::CallRel32(string name):Instruction(name){

}

void CallRel32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 effective_addr(edi); // effective_addr
    const Reg64 mem(rdx);//jit->mem
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。
    
    *stop = true;//call命令では次にどこに飛べば良いかわからず、制御を本体に戻す。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    uint32_t rel32 = jit->Read32(jit->eip);

    this->Push32(code, jit, mem, jit->eip+4);

    //ジャンプ
    #ifdef DEBUG
        code->mov(dword [jit_eip], jit->eip + rel32 + 4);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip = jit->eip + rel32 + 4;
    #else 
        jit->eip = jit->eip + rel32 + 4;
    #endif
    return;
}

Ret32Near::Ret32Near(string name):Instruction(name){

}

void Ret32Near::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 effective_addr(edi); // effective_addr
    const Reg64 mem(rdx);//jit->mem
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。
    const Reg32 data(esi);

    *stop = true;//call命令では次にどこに飛べば良いかわからず、制御を本体に戻す。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    code->mov(mem, (size_t)jit->mem);
    code->mov(jit_eip, (size_t)&jit->eip);

    this->Pop32(code, jit, dword [jit_eip], mem);
    return;
}

PushR32::PushR32(string name):Instruction(name){

}

void PushR32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 effective_addr(edi); // effective_addr
    const Reg64 mem(rdx);//jit->mem
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    code->mov(mem, (size_t)jit->mem);
    REGISTER_KIND register_type = (REGISTER_KIND)(jit->mem[jit->eip]-0x50);
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    switch(register_type){
        case EBP:
            this->Push32(code, jit, mem, jit_ebp);
            break;
        default:
            this->Error("Not implemented: register_type=%d\n", register_type);
    }
    return;
}

Leave::Leave(string name):Instruction(name){

}

void Leave::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 effective_addr(edi); // effective_addr
    const Reg64 mem(rdx);//jit->mem
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    code->mov(mem, (size_t)jit->mem);
    //Leave命令
    //ESPにEBPを格納する
    //スタックからPOPし、EBPに格納する
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    code->mov(jit_esp, jit_ebp);
    this->Pop32(code, jit, jit_ebp, mem);
    return;
}

PushImm8::PushImm8(string name):Instruction(name){

}

void PushImm8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 effective_addr(edi); // effective_addr
    const Reg64 mem(rdx);//jit->mem
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    code->mov(mem, (size_t)jit->mem);

    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    uint32_t imm32 = (int32_t)(int8_t)jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    this->Push32(code, jit, mem, imm32);
    return;
}

PopR32::PopR32(string name):Instruction(name){

}

void PopR32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 effective_addr(edi); // effective_addr
    const Reg64 mem(rdx);//jit->mem
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    code->mov(mem, (size_t)jit->mem);

    REGISTER_KIND register_type = (REGISTER_KIND)(jit->mem[jit->eip]-0x58);
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif

    switch(register_type){
        case EBP:
            this->Pop32(code, jit, jit_ebp, mem);
            break;
        default:
            this->Error("Not implemented: register_type=%d\n", register_type);
    }
    return;
}

AddRm32Imm8::AddRm32Imm8(string name):Instruction(name){

}

void AddRm32Imm8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    uint32_t imm8 = (int32_t)(int8_t)jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    uint32_t rm32;
    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;

    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::CompileStep", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: this->modrm.mod=%d at %s::CompileStep", this->modrm.mod, this->code_name.c_str());

    }else if(this->modrm.mod==1){
        this->Error("Not implemented: this->modrm.mod=%d at %s::CompileStep", this->modrm.mod, this->code_name.c_str());

    }else if(this->modrm.mod==2){
        this->Error("Not implemented: this->modrm.mod=%d at %s::CompileStep", this->modrm.mod, this->code_name.c_str());

    }else if(this->modrm.mod==3){
        REGISTER_KIND register_kind = (REGISTER_KIND)this->modrm.rm;
        switch(register_kind){
            case ESP:
                code->add(jit_esp, imm8);
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

CmpR32Rm32::CmpR32Rm32(string name):Instruction(name){

}

void CmpR32Rm32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg32 effective_addr(edi);     
    const Reg64 mem(rdx);
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。
    Reg32 r32;

    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    this->ParseModRM(jit, code);
    uint32_t rm32;
    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;

    code->mov(mem, (size_t)jit->mem);
    r32 = this->GetReg32ForRegIdx();   
    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::CompileStep", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        this->Error("Not implemented: this->modrm.mod=%d at %s::CompileStep", this->modrm.mod, this->code_name.c_str());
    }else if(this->modrm.mod==1){
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::CompileStep", this->code_name.c_str());
        }
        disp8 = (int32_t)this->modrm.disp8;
        switch ((REGISTER_KIND)this->modrm.rm){
            case EBP:
                code->mov(effective_addr, disp8);
                code->add(effective_addr, jit_ebp);
                break;
            default:
                this->Error("Not implemented: register_type=%d at %s::CompileStep\n", this->modrm.rm, this->code_name.c_str());
                break;
        }
        //jit->mem[effective_addr]++したい。
        code->add(mem, effective_addr);
        code->cmp(r32, dword [mem]);
    }else if(this->modrm.mod==2){
        this->Error("Not implemented: this->modrm.mod=%d at %s::CompileStep", this->modrm.mod, this->code_name.c_str());
    }else if(this->modrm.mod==3){
        this->Error("Not implemented: this->modrm.mod=%d at %s::CompileStep", this->modrm.mod, this->code_name.c_str());
    }
    //TODO:
    //余計なフラグ情報まで更新している。
    //特定のフラグのみを更新するようにすべき。
    //フラグ更新処理
    code->pushfq();
    code->pop(jit_eflags);
    return;
}

JleRel8::JleRel8(string name):Instruction(name){

}

void JleRel8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);//raxをeflagsとして扱う
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    *stop = true;//jmp命令では次にどこに飛べば良いかわからず、制御を本体に戻す。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    int32_t rel8 = (int32_t)(int8_t)jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif

    code->push(jit_eflags);
    code->popfq();
    code->jle("L1");
    code->jmp("L2");
    code->L("L1");
    //code->mov(jit_eip, (size_t)&jit->eip);
    //code->add(dwor [jit_eip], code_size);
    code->mov(jit_eip, (size_t)&jit->eip);//ブロックの終わりの番地を入れたら良いかも?
    code->add(dword [jit_eip], rel8);
    code->L("L2");
    return;
}

Nop::Nop(string name):Instruction(name){

}

void Nop::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    code->nop();
    return;
}

InAlDx::InAlDx(string name):Instruction(name){

}

void InAlDx::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg8   jit_al(r8b);
    const Reg16  jit_dx(r11w);
    const Reg64  jit_eip(rbx);//jit_eipとしてここで扱う。
    
    uint8_t temp_al;
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif

    code->push(rax);
    code->push(rbx);
    code->push(rcx);
    code->push(rdx);
    code->push(r8);
    code->push(r9);
    code->push(r10);
    code->push(r11);
    code->push(r12);
    code->push(r13);
    code->push(r14);
    code->push(r15);

    code->mov(rdi, jit_dx);
    code->call(in8);
    code->mov(rsi, (size_t)&temp_al);
    code->mov(byte [rsi], al);

    code->pop(r15);
    code->pop(r14);
    code->pop(r13);
    code->pop(r12);
    code->pop(r11);
    code->pop(r10);
    code->pop(r9);
    code->pop(r8);
    code->pop(rdx);
    code->pop(rcx);
    code->pop(rbx);
    code->pop(rax);

    code->mov(r8b, byte [rsi]);
    return;
}

OutDxAl::OutDxAl(string name):Instruction(name){

}

void OutDxAl::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg8   jit_al(r8b);
    const Reg16  jit_dx(r11w);
    const Reg64  jit_eip(rbx);//jit_eipとしてここで扱う。
    
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif

    code->push(rax);
    code->push(rbx);
    code->push(rcx);
    code->push(rdx);
    code->push(r8);
    code->push(r9);
    code->push(r10);
    code->push(r11);
    code->push(r12);
    code->push(r13);
    code->push(r14);
    code->push(r15);

    code->mov(rsi, jit_al);
    code->mov(rdi, jit_dx);
    code->call(out8);

    code->pop(r15);
    code->pop(r14);
    code->pop(r13);
    code->pop(r12);
    code->pop(r11);
    code->pop(r10);
    code->pop(r9);
    code->pop(r8);
    code->pop(rdx);
    code->pop(rcx);
    code->pop(rbx);
    code->pop(rax);
    return;
}

MovR8Imm8::MovR8Imm8(string name):Instruction(name){

}

//x64ではhigh byteをサポートしていない。
//例：r8の8bit目から15bit目はない。
void MovR8Imm8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg8 jit_al(r8b); //r8bをjit_alとして扱う。
    const Reg8 jit_bl(r9b); //r8bをjit_alとして扱う。
    const Reg8 jit_cl(r10b); //r8bをjit_alとして扱う。
    const Reg8 jit_dl(r11b); //r8bをjit_alとして扱う。
    const Reg64  jit_eip(rbx);//jit_eipとしてここで扱う。
    uint8_t imm8;
    REGISTER_KIND register_kind = (REGISTER_KIND)(jit->mem[jit->eip]-0xB0);
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    imm8 = jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    switch(register_kind){
        case AL:
            code->mov(jit_al, imm8);
            break;
        case CL:
            code->mov(jit_cl, imm8);
            break;
        case DL:
            code->mov(jit_dl, imm8);
            break;
        case BL:
            code->mov(jit_bl, imm8);
            break;
        case AH:
            code->xor_(ecx, ecx);
            code->mov(ch, imm8);
            code->and_(jit_eax, 0xFFFF00FF);
            code->or_(jit_eax, ecx);
            break;
        default://TODO:r8~15の8bit目~15bit目にアクセスできるものはないらしく、めんどくさいので後回し
            this->Error("Not implemented:: register_kind=%d at %s::CompileStep\n", register_kind, this->code_name.c_str());
    }
    return;
}

CmpAlImm8::CmpAlImm8(string name):Instruction(name){

}

void CmpAlImm8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg8 jit_al(r8b); //r8bをjit_alとして扱う。
    const Reg64  jit_eip(rbx);//jit_eipとしてここで扱う。64bitの理由はjit->eipの番地として扱うから。
    const Reg64 jit_eflags(rax);//eaxをeflagsとして扱う
    uint8_t imm8;
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    imm8 = jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    code->cmp(jit_al, imm8);
    //TODO:
    //余計なフラグ情報まで更新している。
    //特定のフラグのみを更新するようにすべき。
    //フラグ更新処理
    code->pushfq();
    code->pop(jit_eflags);
    return;
}

JzRel8::JzRel8(string name):Instruction(name){

}

void JzRel8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);//raxをeflagsとして扱う
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。

    *stop = true;//jmp命令では次にどこに飛べば良いかわからず、制御を本体に戻す。
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    int32_t rel8 = (int32_t)(int8_t)jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif

    code->push(jit_eflags);
    code->popfq();
    code->jz("L1");
    code->jmp("L2");
    code->L("L1");
    //code->mov(jit_eip, (size_t)&jit->eip);
    //code->add(dwor [jit_eip], code_size);
    code->mov(jit_eip, (size_t)&jit->eip);//ブロックの終わりの番地を入れたら良いかも?
    code->add(dword [jit_eip], rel8);
    code->L("L2");
    return;
}

MovR8Rm8::MovR8Rm8(string name):Instruction(name){

}

void MovR8Rm8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg8  jit_al(r8b);  //r8bをjit_alとして扱う
    const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);//raxをeflagsとして扱う
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。
    const Reg32 effective_addr(edi);     
    const Reg64 mem(rdx);
    Reg8  r8;

    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    this->ParseModRM(jit, code);
    
    code->mov(mem, (size_t)jit->mem);

    uint32_t rm8;
    uint32_t addr;
    uint32_t disp8;
    uint32_t disp32;
    
    switch((REGISTER_KIND)this->modrm.reg_index){
        case EAX:
            r8 = jit_al;
            break;
        default:
            this->Error("Not implemented: (REGISTER_KIND)this->modrm.reg_index=%d at %s::CompileStep", (REGISTER_KIND)this->modrm.reg_index, this->code_name.c_str());
    }
    //TODO:
    //アドレッシングモードは関数化すべき。
    if(this->modrm.mod!=3 && this->modrm.rm==4){
        this->Error("Not implemented: sib at %s::CompileStep", this->code_name.c_str());
    }
    if(this->modrm.mod==0){
        if(this->modrm.rm==5){
            this->Error("Not implemented: %s::CompileStep", this->code_name.c_str());
        }
        if(this->modrm.rm==4){
            this->Error("Not implemented: sib at %s::CompileStep", this->code_name.c_str());
        }
        switch ((REGISTER_KIND)this->modrm.rm){
            case ESI:
                code->mov(effective_addr, jit_esi);
                break;
            default:
                this->Error("Not implemented: register_type=%d at %s::CompileStep\n", this->modrm.rm, this->code_name.c_str());
                break;
        }
        code->add(mem, effective_addr);
        code->mov(r8, byte [mem]);
    }else if(this->modrm.mod==1){
        this->Error("Not implemented: %s::CompileStep", this->code_name.c_str());
    }else if(this->modrm.mod==2){
        this->Error("Not implemented: %s::CompileStep", this->code_name.c_str());
    }else if(this->modrm.mod==3){
        this->Error("Not implemented: %s::CompileStep", this->code_name.c_str());
    }
    return;
}

IncR32::IncR32(string name):Instruction(name){

}

void IncR32::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。
    Reg32 r32;

    REGISTER_KIND register_kind = (REGISTER_KIND)(jit->mem[jit->eip]-0x40);
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif

    switch(register_kind){
        case ESI:
            code->inc(jit_esi);
            break;
        default:
            this->Error("Not implemented: register_kind=%d at %s::CompileStep", register_kind, this->code_name.c_str());
    }
    //TODO:
    //余計なフラグ情報まで更新している。
    //特定のフラグのみを更新するようにすべき。
    //フラグ更新処理
    code->pushfq();
    code->pop(jit_eflags);
    return;
}

IntImm8::IntImm8(string name):Instruction(name){

}

void IntImm8::CompileStep(CodeGenerator* code, bool* stop, Jit* jit){
    const Reg32 jit_eax(r8d);   //r8dをjit_eaxとして扱う。
    const Reg32 jit_ebx(r9d);   //r9dをjit_ebxとして扱う。
    const Reg32 jit_ecx(r10d);  //r10dをjit_ecxとして扱う。
    const Reg32 jit_edx(r11d);  //r11dをjit_edxとして扱う。
    const Reg32 jit_edi(r12d);  //r12dをjit_ediとして扱う。
    const Reg32 jit_esi(r13d);  //r13dをjit_esiとして扱う。
    const Reg32 jit_ebp(r14d);  //r14dをjit_ebpとして扱う。
    const Reg32 jit_esp(r15d);  //r15dをjit_espとして扱う。
    const Reg64 jit_eflags(rax);
    const Reg64 jit_eip(rbx);//jit_eipとしてここで扱う。
    Reg32 r32;
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    uint8_t selector = jit->mem[jit->eip];
    #ifdef DEBUG
        code->inc(dword [jit_eip]);//加算する前の値をコード領域に渡す。そうでないと、2回加算することになる。
        jit->eip += 1;
    #else 
        jit->eip += 1;
    #endif
    code->mov(rsi, selector);
    code->cmp(rsi, 0x10);
    code->je("VIDEO_FUNCTION");
    code->jmp("NOT_IMPLEMENT");
    code->L("VIDEO_FUNCTION");
         /***
        code->push(rax);
        code->push(rbx);
        code->push(rcx);
        code->push(rdx);
        code->push(r8);
        code->push(r9);
        code->push(r10);
        code->push(r11);
        code->push(r12);
        code->push(r13);
        code->push(r14);
        code->push(r15);
        code->mov(rdi, jit_eax);
        code->call(put);
        code->pop(r15);
        code->pop(r14);
        code->pop(r13);
        code->pop(r12);
        code->pop(r11);
        code->pop(r10);
        code->pop(r9);
        code->pop(r8);
        code->pop(rdx);
        code->pop(rcx);
        code->pop(rbx);
        code->pop(rax);
        ***/
        code->jmp("END");
    code->L("NOT_IMPLEMENT");
        code->mov(rdi, selector);
        code->call(bios_error);
    code->L("END");
    return;
}