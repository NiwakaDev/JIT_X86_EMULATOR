#include "Jit.h"
#include "Instruction.h"

Jit::Jit(){
    this->mem = (uint8_t*)malloc(MEM_SIZE);
    this->eip = 0;//内田さんの本では0
    this->save_registers_[ESP] = 0x7C00;
    for(int i=0; i<INSTRUCTION_SIZE; i++){
        this->instructions[i] = NULL;
    }
    this->instructions[0x01] = new AddRm32R32("AddRm32R32");
    this->instructions[0x83] = new Code83("Code83");
    this->instructions[0x89] = new MovRm32R32("MovRm32R32");
    this->instructions[0x8B] = new MovR32Rm32("MovR32Rm32");
    for(int i=0; i<REGISTER_KIND_TOTAL; i++){
        this->instructions[0xB8+i] = new MovR32Imm32("MovR32Imm32");
    }
    this->instructions[0xC7] = new MovRm32Imm32("MovRm32Imm32");
    this->instructions[0xE9] = new JmpRel32("JmpRel32");
    this->instructions[0xEB] = new JmpRel8("JmpRel8");
    this->instructions[0xFF] = new CodeFF("CodeFF");
    for(int i=0; i<MEM_SIZE; i++){
        this->eip2code[i] = NULL;
    }
}

//TODO:セグメントフォールトが起こる可能性があるが、今の所は無視
uint32_t Jit::Read32(uint32_t addr){
    return *(uint32_t*)(this->mem+addr);
}

Xbyak::CodeGenerator* Jit::CompileBlock(){
    using namespace Xbyak::util;
	using namespace Xbyak;
    Xbyak::CodeGenerator* code = new Xbyak::CodeGenerator();
	const Reg32 jit_eax(r8d); //r8dをjit_eaxとして扱う。
	const Reg32 jit_ebx(r9d); //r9dをjit_ebxとして扱う。
	const Reg32 jit_ecx(r10d);//r10dをjit_ecxとして扱う。
	const Reg32 jit_edx(r11d);//r11dをjit_edxとして扱う。
	const Reg32 jit_edi(r12d);//r12dをjit_ediとして扱う。
	const Reg32 jit_esi(r13d);//r13dをjit_esiとして扱う。
	const Reg32 jit_ebp(r14d);//r14dをjit_ebpとして扱う。
	const Reg32 jit_esp(r15d);//r15dをjit_espとして扱う。
    const Reg32 jit_eflags(eax);//eaxをeflagsとして扱う
    const Reg64 save_registers(rbx);//これは番地として扱うので、64bitレジスタ

    bool stop = false;
    code->push(rax);
    code->push(rbx);
    code->push(rcx);
    code->push(rdx);
    code->push(rdi);
    code->push(rsi);
    code->push(rbp);
    code->push(rsp);
    code->push(r8);
    code->push(r9);
    code->push(r10);
    code->push(r11);
    code->push(r12);
    code->push(r13);
    code->push(r14);
    code->push(r15);

	code->mov(jit_eax, this->save_registers_[EAX]);
	code->mov(jit_esp, this->save_registers_[ESP]);
    code->mov(jit_ebp, this->save_registers_[EBP]);
    code->mov(jit_esi, this->save_registers_[ESI]);
    code->mov(jit_edi, this->save_registers_[EDI]);

    code->mov(jit_eflags, this->eflags.raw);
    while(!stop){
        uint8_t op_code = this->mem[this->eip];
        if(this->instructions[op_code]==NULL){
            fprintf(stderr, "Not implemented: op_code=0x%02X at Jit::CompileBlock\n", op_code);
            exit(1);
        }
        this->instructions[op_code]->CompileStep(code, &stop, this);
    }
    //eaxを保存
    code->mov(save_registers, (size_t)&this->save_registers_);
    code->mov(dword [save_registers], jit_eax);
    //espを保存
    code->mov(rcx, 4);
    code->mov(dword [save_registers+rcx*4], jit_esp);
    //ebpを保存
    code->mov(rcx, 5);
    code->mov(dword [save_registers+rcx*4], jit_ebp);
    //esiを保存
    code->mov(rcx, 6);
    code->mov(dword [save_registers+rcx*4], jit_esi);
    //ediを保存
    code->mov(rcx, 7);
    code->mov(dword [save_registers+rcx*4], jit_edi);
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
    code->pop(rsp);
    code->pop(rbp);
    code->pop(rsi);
    code->pop(rdi);
    code->pop(rdx);
    code->pop(rcx);
    code->pop(rbx);
    code->pop(rax);
    code->ret();
    return code;

}

bool Jit::IsCompiledBlock(uint32_t eip){
    return this->eip2code[eip]!=NULL;
}

void Jit::Run(){
    Xbyak::CodeGenerator* code;
    if(this->IsCompiledBlock(this->eip)){
        code = this->eip2code[this->eip];
    }else{
        uint32_t first_eip = this->eip;
        code = this->CompileBlock();
        this->eip2code[first_eip] = code;
    }
    fprintf(stderr, "before:\n");
    fprintf(stderr, "eax   = 0x%08X\n", this->save_registers_[EAX]);
    fprintf(stderr, "ecx   = 0x%08X\n", this->save_registers_[ECX]);
    fprintf(stderr, "edx   = 0x%08X\n", this->save_registers_[EDX]);
    fprintf(stderr, "ebx   = 0x%08X\n", this->save_registers_[EBX]);
    fprintf(stderr, "esp   = 0x%08X\n", this->save_registers_[ESP]);
    fprintf(stderr, "ebp   = 0x%08X\n", this->save_registers_[EBP]);
    fprintf(stderr, "esi   = 0x%08X\n", this->save_registers_[ESI]);
    fprintf(stderr, "edi   = 0x%08X\n", this->save_registers_[EDI]);
    fprintf(stderr, "eip   = 0x%08X\n", this->eip);
    //fprintf(stderr, "eflags= 0x%08X\n", this->eflags.raw);eflagsはまだ本で登場しない
    void (*f)() = (void (*)())code->getCode();
    f();
    fprintf(stderr, "after:\n");
    fprintf(stderr, "eax   = 0x%08X\n", this->save_registers_[EAX]);
    fprintf(stderr, "ecx   = 0x%08X\n", this->save_registers_[ECX]);
    fprintf(stderr, "edx   = 0x%08X\n", this->save_registers_[EDX]);
    fprintf(stderr, "ebx   = 0x%08X\n", this->save_registers_[EBX]);
    fprintf(stderr, "esp   = 0x%08X\n", this->save_registers_[ESP]);
    fprintf(stderr, "ebp   = 0x%08X\n", this->save_registers_[EBP]);
    fprintf(stderr, "esi   = 0x%08X\n", this->save_registers_[ESI]);
    fprintf(stderr, "edi   = 0x%08X\n", this->save_registers_[EDI]);
    fprintf(stderr, "eip   = 0x%08X\n", this->eip);
    //fprintf(stderr, "eflags= 0x%08X\n", this->eflags.raw);
}
