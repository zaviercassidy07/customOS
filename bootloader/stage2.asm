[org 0x0000] ; no offset for this one
[bits 16]

xor ax, ax ; clear ax and es
mov es, ax
mov bx, 0x9000

mov ah, 0x02 ; read operation
mov al, 8 ; amount to read
mov ch, 0 ; cylinder
mov cl, 6 ; sector
mov dh, 0 ; head

int 0x13

jmp 0x0900:0000 ; long range jump, it uses x:y = x*16 + y
; idk why, apparently it just likes long jump more in this context

times 2048 - ($ - $$) db 0 ; pad code