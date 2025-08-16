.data
str_1: .asciiz "hello "
str_2: .asciiz "!\n"
str_0: .asciiz "please tell me your name: "

# Buffers for string operations
input_buffer: .space 256
concat_buffer: .space 512
repeat_buffer: .space 1024

.text
.globl main
main:
    addiu $sp, $sp, -12
    move $fp, $sp
    la $a0, str_0
    li $v0, 4
    syscall
    li $v0, 8
    la $a0, input_buffer
    li $a1, 256
    syscall
    la $t0, input_buffer
    move $t1, $t0
len_scan_assign1:
    lb $t2, 0($t1)
    beq $t2, $zero, len_done_assign1
    addiu $t1, $t1, 1
    j len_scan_assign1
len_done_assign1:
    subu $t3, $t1, $t0
    addiu $a0, $t3, 1
    li $v0, 9
    syscall
    move $t4, $v0
    move $t5, $t0
copy_loop_assign1:
    lb $t6, 0($t5)
    sb $t6, 0($t4)
    beq $t6, $zero, copy_done_assign1
    addiu $t5, $t5, 1
    addiu $t4, $t4, 1
    j copy_loop_assign1
copy_done_assign1:
    subu $a0, $t4, $t3
    jal string_trim_newline
    subu $a0, $t4, $t3
    sw $a0, 0($fp)
    la $a0, str_1
    addiu $sp, $sp, -4
    sw $a0, 0($sp)
    lw $a0, 0($fp)
    lw $a1, 0($sp)
    jal string_concat
    addiu $sp, $sp, 4
    addiu $sp, $sp, -4
    sw $a0, 0($sp)
    la $a0, str_2
    lw $a1, 0($sp)
    jal string_concat
    addiu $sp, $sp, 4
    li $v0, 4
    syscall
    addiu $sp, $sp, 12
    li $v0, 10
    syscall

# String utility functions

.text
string_concat:
    # $a0 = second string address, $a1 = first string address
    # result in $a0
    move $t0, $a1        # first string address
    move $t1, $a0        # second string address
    la $a0, concat_buffer # result buffer
    move $t2, $a0        # current position in result

concat_loop1:
    lb $t3, 0($t0)       # load byte from first string
    beq $t3, $zero, concat_second # if null terminator, copy second string
    sb $t3, 0($t2)       # store byte to result
    addiu $t0, $t0, 1    # next char in first string
    addiu $t2, $t2, 1    # next position in result
    j concat_loop1

concat_second:
concat_loop2:
    lb $t3, 0($t1)       # load byte from second string
    sb $t3, 0($t2)       # store byte to result
    beq $t3, $zero, concat_done # if null terminator, done
    addiu $t1, $t1, 1    # next char in second string
    addiu $t2, $t2, 1    # next position in result
    j concat_loop2

concat_done:
    jr $ra               # return

string_repeat:
    # $a1 = string address, $a2 = repeat count
    # result in $a0
    la $a0, repeat_buffer # result buffer
    move $t0, $a0        # current position in result
    move $t1, $a2        # repeat counter

repeat_outer_loop:
    beq $t1, $zero, repeat_done # if counter is 0, done
    move $t2, $a1        # reset string pointer

repeat_inner_loop:
    lb $t3, 0($t2)       # load byte from string
    beq $t3, $zero, repeat_next # if null terminator, next iteration
    sb $t3, 0($t0)       # store byte to result
    addiu $t2, $t2, 1    # next char in string
    addiu $t0, $t0, 1    # next position in result
    j repeat_inner_loop

repeat_next:
    addiu $t1, $t1, -1   # decrement counter
    j repeat_outer_loop

repeat_done:
    sb $zero, 0($t0)     # null terminate result
    jr $ra               # return

string_trim_newline:
    # Trim trailing newline from string at address in $a0
    move $t0, $a0         # load buffer address from $a0

trim_loop:
    lb $t1, 0($t0)        # load current character
    beq $t1, $zero, trim_done # if null terminator, done
    li $t2, 10            # ASCII code for newline (\n)
    beq $t1, $t2, trim_newline # if newline, remove it
    li $t2, 13            # ASCII code for carriage return (\r)
    beq $t1, $t2, trim_newline # if carriage return, remove it
    addiu $t0, $t0, 1     # next character
    j trim_loop

trim_newline:
    sb $zero, 0($t0)      # replace newline with null terminator

trim_done:
    jr $ra                # return

