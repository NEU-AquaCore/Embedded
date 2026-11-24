.syntax unified
.cpu cortex-m3
.thumb

.global Reset_Handler
.global Default_Handler
.extern SysTick_Handler

/* Cortex-M3 核心向量 + STM32F103 中容量器件的 43 个外设中断。 */
.section .isr_vector, "a", %progbits
.balign 4
.global g_pfnVectors
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
    .word SysTick_Handler  /* SysTick：第 15 项，偏移 0x3C。 */

    /* 本实验不启用外设中断；后续实验需替换对应项。 */
    .rept 43
    .word Default_Handler
    .endr
.size g_pfnVectors, . - g_pfnVectors

.section .text.Reset_Handler, "ax", %progbits
.type Reset_Handler, %function
.thumb_func
Reset_Handler:
    /* 将有初始值的全局/静态变量从 Flash 复制到 RAM。 */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b

    /* 清零未初始化的全局/静态变量，包括毫秒计数器。 */
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:
    cmp r1, r2
    bcs 4f
    str r3, [r1], #4
    b 3b
4:
    bl main
5:
    b 5b
.size Reset_Handler, . - Reset_Handler

.section .text.Default_Handler, "ax", %progbits
.type Default_Handler, %function
.thumb_func
Default_Handler:
    b Default_Handler
.size Default_Handler, . - Default_Handler

.section .note.GNU-stack, "", %progbits
