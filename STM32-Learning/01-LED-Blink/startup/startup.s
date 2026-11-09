.syntax unified
.cpu cortex-m3
.thumb

.global Reset_Handler
.global Default_Handler

/* 初始栈顶地址 */
.word _estack

.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object

g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word Default_Handler  /* NMI */
    .word Default_Handler  /* HardFault */
    .word Default_Handler  /* MemManage */
    .word Default_Handler  /* BusFault */
    .word Default_Handler  /* UsageFault */
    .word 0
    .word 0
    .word 0
    .word 0
    .word Default_Handler  /* SVCall */
    .word Default_Handler  /* Debug Monitor */
    .word 0
    .word Default_Handler  /* PendSV */
    .word Default_Handler  /* SysTick */

/* Reset Handler */
.section .text.Reset_Handler
.type Reset_Handler, %function

Reset_Handler:
    /* 初始化 .data 段 */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

copy_data:
    cmp r1, r2
    bcc copy_data_loop
    b zero_bss

copy_data_loop:
    ldr r3, [r0]
    str r3, [r1]
    adds r0, #4
    adds r1, #4
    b copy_data

zero_bss:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

zero_bss_loop:
    cmp r1, r2
    bcs call_main
    str r3, [r1]
    adds r1, #4
    b zero_bss_loop

call_main:
    bl main

hang:
    b hang

/* 默认异常处理 */
.section .text.Default_Handler
.type Default_Handler, %function

Default_Handler:
    b Default_Handler