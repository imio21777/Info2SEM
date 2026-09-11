# Test asm for MIPS-LITE2
	.text
initial:
	beq $a0,$zero,main
	ori $s1,0xff
	ori $s2,0xff
	ori $s3,0xff
	lui $s1,0x17
	ori $s1,0x73
	lui $s2,0x62
	ori $s2,0x69
	lui $s3,0x56
	ori $s3,0xcd
	sw $s1,-12($a0)
	sw $s2,-8($a0)
	sw $s3,-4($a0)
	jr $ra
main:
	addu $a0,$zero,12
	jal initial
	addu $a0,$zero,4
	addu $s7,$zero,1024
loop:
	subu $s7,$s7,1
	jal func
	addu $a0,$a0,4
	beq $s7,$zero,end
	j loop
func:
	lw $t0, -4($a0)
	lw $t1, 0($a0)
	lw $t2, 4($a0)
	lw $t3,8($a0)
	lw $t4,12($a0)
	ori $t7,$t0,0x01
	beq $t7,$zero,else1
	addu $t1,$t1,$t2
	subu $t1,$t1,$t3
	addu $t3,$t3,$t4
	subu $t4,$t2,$t4
else1:
	addu $t3,$t3,$t2
	subu $t4,$t1,$t4
	ori $t7,$t0,0x80
	beq $t7,$zero,else2
	ori $t1,0xAD
	ori $t2,0xF3
	ori $t3,0x2B
	ori $t4,0x17
else2:
	sw $t1,4($a0)
	sw $t2,8($a0)
	sw $t3,12($a0)
	sw $t4,16($a0)
	jr $ra
end:
	ori $v0, $zero, 10
	syscall
