#include <stdint.h>

/* RCC 寄存器地址 */
#define RCC_APB2ENR  (*(volatile uint32_t *)0x40021018)

/* GPIOC 寄存器地址 */
#define GPIOC_CRH    (*(volatile uint32_t *)0x40011004)
#define GPIOC_BRR    (*(volatile uint32_t *)0x40011014)
#define GPIOC_BSRR   (*(volatile uint32_t *)0x40011010)

/* PC13 LED */
#define LED_PIN      (1U << 13)

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm volatile ("nop");
    }
}

int main(void)
{
    /* 1. 开启 GPIOC 时钟 */
    RCC_APB2ENR |= (1U << 4);

    /* 2. 配置 PC13 为推挽输出，2MHz */
    GPIOC_CRH &= ~(0xFU << 20);
    GPIOC_CRH |=  (0x2U << 20);

    while (1)
    {
        /* 3. PC13 输出低电平，LED 亮 */
        GPIOC_BRR = LED_PIN;

        delay(1000000);

        /* 4. PC13 输出高电平，LED 灭 */
        GPIOC_BSRR = LED_PIN;

        delay(1000000);
    }
}