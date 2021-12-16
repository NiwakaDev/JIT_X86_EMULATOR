# NIWAKA_JIT_X86
これはにわかなjit化されたx86エミュレータです。

modrm-test.binというプログラムが動きました。

```
BITS 32
    org 0x7c00
    sub esp, 16
    mov ebp, esp
    mov eax, 2
    mov dword [ebp+4], 5
    add dword [ebp+4], eax
    mov esi, [ebp+4]
    inc dword [ebp+4]
    mov edi, [ebp+4]
    jmp 0

```

![modrm-test.binの実行結果](https://github.com/NiwakaDev/NIWAKA_JIT_X86/blob/main/images/%E3%82%B9%E3%82%AF%E3%83%AA%E3%83%BC%E3%83%B3%E3%82%B7%E3%83%A7%E3%83%83%E3%83%88%202021-12-16%208.37.02.png)

call-test.binというプログラムが動きました。

```
BITS 32
    org 0x7c00
start:
    mov eax, 0x00f1
    mov ebx, 0x0029
    call add_routine
    jmp 0
add_routine:
    mov ecx, eax
    add ecx, ebx
    ret

```

![modrm-test.binの実行結果](https://github.com/NiwakaDev/NIWAKA_JIT_X86/blob/main/images/call_test.png)

<h2>目標</h2>
 <a href="https://www.amazon.co.jp/%E8%87%AA%E4%BD%9C%E3%82%A8%E3%83%9F%E3%83%A5%E3%83%AC%E3%83%BC%E3%82%BF%E3%81%A7%E5%AD%A6%E3%81%B6x86%E3%82%A2%E3%83%BC%E3%82%AD%E3%83%86%E3%82%AF%E3%83%81%E3%83%A3-%E3%82%B3%E3%83%B3%E3%83%94%E3%83%A5%E3%83%BC%E3%82%BF%E3%81%8C%E5%8B%95%E3%81%8F%E4%BB%95%E7%B5%84%E3%81%BF%E3%82%92%E5%BE%B9%E5%BA%95%E7%90%86%E8%A7%A3-%E5%86%85%E7%94%B0%E5%85%AC%E5%A4%AA/dp/4839954747/ref=sr_1_1?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&keywords=%E8%87%AA%E4%BD%9C%E3%82%A8%E3%83%9F%E3%83%A5%E3%83%AC%E3%83%BC%E3%82%BF&qid=1638354148&s=books&sr=1-1">自作エミュレータで学ぶx86アーキテクチャ-コンピュータが動く仕組みを徹底理解! 
 内田公太 、 上川大介</a>
 
 この本の4.2章までやる。
 
 <h2>jitライブラリ</h2>
 https://github.com/herumi/xbyak