#include "Jit.h"
#include "Instruction.h"

using namespace Xbyak::util;
using namespace Xbyak;

static const Xbyak::Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
static const Xbyak::Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
static const Xbyak::Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
static const Xbyak::Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
static const Xbyak::Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
static const Xbyak::Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
static const Xbyak::Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
static const Xbyak::Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
static const Xbyak::Reg32 jit_eflags(eax);//eaxをeflagsとして扱う
static const Xbyak::Reg64 jit_eip(rbx);//これは番地として扱うので、64bitレジスタ
static const Xbyak::Reg64 save_registers(rcx);//これは番地として扱うので、64bitレジスタ


Jit::Jit(){
    this->mem = (uint8_t*)malloc(MEM_SIZE);
    this->eip = 0;//内田さんの本では0
    this->save_registers_[ESP] = 0x7C00;
    for(int i=0; i<INSTRUCTION_SIZE; i++){
        this->instructions[i] = NULL;
    }
    this->instructions[0x01] = new AddRm32R32("AddRm32R32");
    this->instructions[0x3B] = new CmpR32Rm32("CmpR32Rm32");
    this->instructions[0x3C] = new CmpAlImm8("CmpAlImm8");
    for(int i=0; i<REGISTER_KIND_TOTAL; i++){
        this->instructions[0x40+i] = new IncR32("IncR32");
    }
    for(int i=0; i<REGISTER_KIND_TOTAL; i++){
        this->instructions[0x50+i] = new PushR32("PushR32");
    }
    for(int i=0; i<REGISTER_KIND_TOTAL; i++){
        this->instructions[0x58+i] = new PopR32("PopR32");
    }
    this->instructions[0x6A] = new PushImm8("PushImm8");
    this->instructions[0x74] = new JzRel8("JzRel8");
    this->instructions[0x7E] = new JleRel8("JleRel8");
    this->instructions[0x81] = new Code81("Code81");
    this->instructions[0x83] = new Code83("Code83");
    this->instructions[0x89] = new MovRm32R32("MovRm32R32");
    this->instructions[0x8A] = new MovR8Rm8("MovR8Rm8");
    this->instructions[0x8B] = new MovR32Rm32("MovR32Rm32");
    this->instructions[0x90] = new Nop("Nop");
    for(int i=0; i<REGISTER_KIND_TOTAL; i++){
        this->instructions[0xB0+i] = new MovR8Imm8("MovR8Imm8");
    }
    for(int i=0; i<REGISTER_KIND_TOTAL; i++){
        this->instructions[0xB8+i] = new MovR32Imm32("MovR32Imm32");
    }
    this->instructions[0xC3] = new Ret32Near("Ret32Near");
    this->instructions[0xC7] = new MovRm32Imm32("MovRm32Imm32");
    this->instructions[0xC9] = new Leave("Leave");
    this->instructions[0xCD] = new IntImm8("IntImm8");
    this->instructions[0xE8] = new CallRel32("CallRel32");
    this->instructions[0xE9] = new JmpRel32("JmpRel32");
    this->instructions[0xEB] = new JmpRel8("JmpRel8");
    this->instructions[0xEC] = new InAlDx("InAlDx");
    this->instructions[0xEE] = new OutDxAl("OutDxAl");
    this->instructions[0xFF] = new CodeFF("CodeFF");
    for(int i=0; i<MEM_SIZE; i++){
        this->eip2block[i] = NULL;
    }
}

void Jit::ShowRegisters(){
    fprintf(stderr, "eax   = 0x%08X\n", this->save_registers_[EAX]);
    fprintf(stderr, "ecx   = 0x%08X\n", this->save_registers_[ECX]);
    fprintf(stderr, "edx   = 0x%08X\n", this->save_registers_[EDX]);
    fprintf(stderr, "ebx   = 0x%08X\n", this->save_registers_[EBX]);
    fprintf(stderr, "esp   = 0x%08X\n", this->save_registers_[ESP]);
    fprintf(stderr, "ebp   = 0x%08X\n", this->save_registers_[EBP]);
    fprintf(stderr, "esi   = 0x%08X\n", this->save_registers_[ESI]);
    fprintf(stderr, "edi   = 0x%08X\n", this->save_registers_[EDI]);
    fprintf(stderr, "eip   = 0x%08X\n", this->eip);
}

//TODO:セグメントフォールトが起こる可能性があるが、今の所は無視
uint32_t Jit::Read32(uint32_t addr){
    return *(uint32_t*)(this->mem+addr);
}

Xbyak::CodeGenerator* Jit::CompileBlock(){
    CodeGenerator* code = new CodeGenerator(128, AutoGrow);

    bool stop = false;
    code->push(rbp);
    code->mov(rbp, rsp);
    code->push(rax);
    code->push(rbx);
    code->push(rcx);
    code->push(rdx);
    code->push(rdi);
    code->push(rsi);
    //code->push(rbp);
    //code->push(rsp);
    code->push(r8);
    code->push(r9);
    code->push(r10);
    code->push(r11);
    code->push(r12);
    code->push(r13);
    code->push(r14);
    code->push(r15);

    //何故、即値を入れないか?
    //->1度目の実行時の値が固定されては困るから。
    code->mov(save_registers, (size_t)&this->save_registers_);
    code->mov(jit_eax, dword [save_registers]);    //eaxをjit_eaxに割り当て
    code->mov(jit_ecx, dword [save_registers+1*4]);//ecxをjit_eaxに割り当て
    code->mov(jit_edx, dword [save_registers+2*4]);//edxをjit_eaxに割り当て
    code->mov(jit_ebx, dword [save_registers+3*4]);//ebxをjit_eaxに割り当て
    code->mov(jit_esp, dword [save_registers+4*4]);//espをjit_eaxに割り当て
    code->mov(jit_ebp, dword [save_registers+5*4]);//ebpをjit_eaxに割り当て
    code->mov(jit_esi, dword [save_registers+6*4]);//esiをjit_eaxに割り当て
    code->mov(jit_edi, dword [save_registers+7*4]);//ediをjit_eaxに割り当て
    code->mov(jit_eip, (size_t)&this->eip);

    //code->mov(jit_eip, this->eip);
    //eflagsを保存
    code->mov(save_registers, (size_t)&this->eflags.raw);
    code->mov(jit_eflags, dword [save_registers]);

    while(!stop){
        uint8_t op_code = this->mem[this->eip];
        if(this->instructions[op_code]==NULL){
            fprintf(stderr, "Not implemented: op_code=0x%02X at Jit::CompileBlock\n", op_code);
            exit(1);
        }
        this->instructions[op_code]->CompileStep(code, &stop, this);
        #ifdef DEBUG
            stop = true;//デバッグ時は1つの機械語命令でコンパイルを終了。
        #else 
            //stop = false;//通常時でも同じく。理由はREADME.mdに書いてます。
        #endif
    }
    //eaxを保存
    code->mov(save_registers, (size_t)&this->save_registers_);
    code->mov(dword [save_registers], jit_eax);
    //ecxを保存
    code->mov(dword [save_registers+1*4], jit_ecx);
    //edxを保存
    code->mov(dword [save_registers+2*4], jit_edx);
    //ebxを保存
    code->mov(dword [save_registers+3*4], jit_ebx);
    //espを保存
    code->mov(dword [save_registers+4*4], jit_esp);
    //ebpを保存
    code->mov(dword [save_registers+5*4], jit_ebp);
    //esiを保存
    code->mov(dword [save_registers+6*4], jit_esi);
    //ediを保存
    code->mov(dword [save_registers+7*4], jit_edi);
    //eflagsを保存
    code->mov(save_registers, (size_t)&this->eflags.raw);
    code->mov(dword [save_registers], jit_eflags);
    
    code->pop(r15);
    code->pop(r14);
    code->pop(r13);
    code->pop(r12);
    code->pop(r11);
    code->pop(r10);
    code->pop(r9);
    code->pop(r8);
    code->pop(rsi);
    code->pop(rdi);
    code->pop(rdx);
    code->pop(rcx);
    code->pop(rbx);
    code->pop(rax);
    code->leave();
    code->ret();
    return code;

}

bool Jit::IsCompiledBlock(uint32_t eip){
    return this->eip2block[eip]!=NULL;
}

void ToBinary(CodeGenerator* code){
    FILE* binary = fopen("binary.bin", "wb");
    fwrite(code->getCode(), 1, code->getSize(), binary);
    fclose(binary);
}

void Jit::Run(){
    CodeGenerator* code;
    Block* block;
    if(this->IsCompiledBlock(this->eip)){
        block = this->eip2block[this->eip];
    }else{
        uint32_t first_eip = this->eip;
        code = this->CompileBlock();
        code->ready();
        block = (void (*)())code->getCode();
        this->eip2block[first_eip] = block;
        this->eip = first_eip;
    }
    //code->ready();
    //void (*f)() = (void (*)())code->getCode();
    block();
}
