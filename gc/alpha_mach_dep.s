        .text
        .align  4
        .globl  GC_push_regs
        .ent    GC_push_regs 2
GC_push_regs:
	ldgp    $gp, 0($27)
	lda     $sp, -32($sp)
	stq     $26, 8($sp)
        .mask   0x04000000, -8
        .frame  $sp, 16, $26, 0

        lda   $16, 0( $9 ); jsr   $26, GC_push_one; ldgp  $gp, 0($26) 
        lda   $16, 0( $10 ); jsr   $26, GC_push_one; ldgp  $gp, 0($26) 
        lda   $16, 0( $11 ); jsr   $26, GC_push_one; ldgp  $gp, 0($26) 
        lda   $16, 0( $12 ); jsr   $26, GC_push_one; ldgp  $gp, 0($26) 
        lda   $16, 0( $13 ); jsr   $26, GC_push_one; ldgp  $gp, 0($26) 
        lda   $16, 0( $14 ); jsr   $26, GC_push_one; ldgp  $gp, 0($26) 

        lda   $16, 0( $15 ); jsr   $26, GC_push_one; ldgp  $gp, 0($26) 

        lda   $16, 0( $29 ); jsr   $26, GC_push_one; ldgp  $gp, 0($26) 

	ldq     $26, 8($sp)
	lda     $sp, 32($sp)
	ret     $31, ($26), 1
	.end    GC_push_regs
