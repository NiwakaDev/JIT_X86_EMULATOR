# NIWAKA_JIT_X86
これはにわかなjit化されたx86エミュレータです。

![select](https://github.com/NiwakaDev/NIWAKA_JIT_X86/blob/main/images/select.png)


上の画像のプログラム:
```
BITS 32
    org 0x7c00
start:
    mov edx, 0x03f8
mainloop:
    mov al, '>'    ; プロンプトを表示
    out dx, al
input:
    in al, dx       ; 1文字入力
    cmp al, 'h'
    je puthello     ; hならhelloを表示
    cmp al, 'w'
    je putworld     ; wならworldを表示
    cmp al, 'q'
    je fin          ; qなら終了
    jmp input       ; それ以外なら再入力
puthello:
    mov esi, msghello
    call puts
    jmp mainloop
putworld:
    mov esi, msgworld
    call puts
    jmp mainloop
fin:
    jmp 0

; esiに設定された文字列を表示するサブルーチン
puts:
    mov al, [esi]
    inc esi
    cmp al, 0
    je putsend
    out dx, al
    jmp puts
putsend:
    ret

msghello:
    db "hello", 0x0d, 0x0a, 0
msgworld:
    db "world", 0x0d, 0x0a, 0

```


4.2章の以下のコードが動きます。(subroutine32.bin)

```
BITS 32
    org 0x7c00
start:            ; プログラムの開始
    mov esi, msg
    call puts     ; サブルーチンを呼び出す
    jmp 0

puts:
    mov al, [esi]  ; 1文字読み込む
    inc esi
    cmp al, 0     ; 文字列の末尾
    je  puts_end  ; に来たら終了
    mov ah, 0x0e
    mov ebx, 15
    int 0x10      ; BIOS を呼び出す
    jmp puts
puts_end:
    ret           ; サブルーチンから抜ける

msg:
    db "hello, world", 0x0d, 0x0a, 0
```

 
 <h2>ビルド</h2>

```
make
```

<h2>実行</h2>

```
./x86 subroutine32.bin
```

or

```
./x86 select.bin
```

 <h2>サンプルプログラム</h2>
  <a href="https://www.amazon.co.jp/%E8%87%AA%E4%BD%9C%E3%82%A8%E3%83%9F%E3%83%A5%E3%83%AC%E3%83%BC%E3%82%BF%E3%81%A7%E5%AD%A6%E3%81%B6x86%E3%82%A2%E3%83%BC%E3%82%AD%E3%83%86%E3%82%AF%E3%83%81%E3%83%A3-%E3%82%B3%E3%83%B3%E3%83%94%E3%83%A5%E3%83%BC%E3%82%BF%E3%81%8C%E5%8B%95%E3%81%8F%E4%BB%95%E7%B5%84%E3%81%BF%E3%82%92%E5%BE%B9%E5%BA%95%E7%90%86%E8%A7%A3-%E5%86%85%E7%94%B0%E5%85%AC%E5%A4%AA/dp/4839954747/ref=sr_1_1?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&keywords=%E8%87%AA%E4%BD%9C%E3%82%A8%E3%83%9F%E3%83%A5%E3%83%AC%E3%83%BC%E3%82%BF&qid=1638354148&s=books&sr=1-1">自作エミュレータで学ぶx86アーキテクチャ-コンピュータが動く仕組みを徹底理解! 
 内田公太 、 上川大介</a>からです。

 <h2>jitライブラリ</h2>
 https://github.com/herumi/xbyak

 <h2>問題点</h2>
 呼び出し規約の関係で、Windowsでは動作しません。
 このJITエミュレータは、
 １つの機械語命令をコンパイルするたびに、制御を本体に移しています。
 つまり、めちゃくちゃ遅いです。これに関しては、今後改良する予定です。
 何故こんなことしたのかと言いますと、動かし切ることをまずは目指したからです。