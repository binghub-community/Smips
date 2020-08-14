main:
    li   $t0, 10
loop:
    add $a0, $t0, $a0
    li   $v0, 1
    syscall
    li   $a0, '\n'
    li   $v0, 11
    syscall
    add  $t0, $t0, -1
    bne  $t0, $0, loop
    or   $s0, $t0, $t1
    jr   $ra
