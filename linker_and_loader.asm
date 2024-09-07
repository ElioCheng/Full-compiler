.import printHex
.import readWord

lis $4
.word 4
lis $10
.word readWord
lis $12
.word printHex


lis $24
.word 12

lis $18
.word 0x100000 
lis $19
.word 0x10000

add $30, $18, $19 
add $30, $30, $4

sw $31, 0($30) ;  store the return value of our own program before calling jalr

jalr $10 ; read the value of a
add $16, $3, $0 ; storing the value of a into $16
sw $16, 4($30) ; store a on the stack

jalr $10 ; doing read for three times
jalr $10 
add $17, $3, $0 

jalr $10 ; after we read the last line, the return value is the size of the MERL file

sub $5, $3, $4 ; i-=12 subtract the three headers now we have the "actual" size 
sub $5, $5, $4
sub $5, $5, $4 
sw $5, 8($30) ; store the code size on the stack

add $20, $0, $0 ; setup our offset counter

while:
slt $7, $20, $5 ; if statement for the counter
beq $7, $0, endFirstWhile

jalr $10 ; reads
add $1, $3, $0 ; doing copy

add $13, $20, $16 ; add a and the offset counter 
sw $1, 0($13) ; store the value we just read to the memory address

jalr $12 ; prints
add $20, $20, $4 ; increase the counter so that we loads the next line
beq $0, $0, while

endFirstWhile:

; increment the code size by 12
add $15, $5, $4
add $15, $15, $4
add $15, $15, $4

lastWhile:

slt $7, $15, $17
beq $7, $0, endLastWhile

jalr $10
jalr $10

add $20, $16, $3
sub $20, $20, $4
sub $20, $20, $4
sub $20, $20, $4

lw $21, 0($20)
add $21, $21, $16
sub $21, $21, $4
sub $21, $21, $4
sub $21, $21, $4

sw $21, 0($20)

add $15, $15, $4
add $15, $15, $4

beq $0, $0, lastWhile

endLastWhile:

jalr $16 ; jump to the file we just loaded

lis $4
.word 4
lis $10
.word readWord
lis $12
.word printHex

lis $18
.word 0x100000 
lis $19
.word 0x10000

add $30, $18, $19 
add $30, $30, $4

lw $5, 8($30) 
lw $16, 4($30) 

add $6, $0, $0 ; reset the counter 

secondWhile:
slt $7, $6, $5
beq $7, $0, endCode

add $23, $16, $6 
lw $1, 0($23) 
jalr $12 ; prints

add $6, $6, $4 ; increase the counter

beq $0, $0, secondWhile

endCode:
lw $31, 0($30)
jr $31
