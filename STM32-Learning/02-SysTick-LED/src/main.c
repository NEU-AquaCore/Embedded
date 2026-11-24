#include <stdint.h>
#include "timing.h"

/* 本实验主动选择内部 HSI：标称 8MHz，不依赖外部晶振。 */
#define CORE_CLOCK_HZ       8000000U
#define LED_INTERVAL_MS     500U
#define LED_PIN             (1U << 13)

#define RCC_CR              (*(volatile uint32_t *)0x40021000U)
#define RCC_CFGR            (*(volatile uint32_t *)0x40021004U)
#define RCC_APB2ENR         (*(volatile uint32_t *)0x40021018U)
#define GPIOC_CRH           (*(volatile uint32_t *)0x40011004U)
#define GPIOC_BSRR          (*(volatile uint32_t *)0x40011010U)
#define GPIOC_BRR           (*(volatile uint32_t *)0x40011014U)

#define SCB_VTOR            (*(volatile uint32_t *)0xE000ED08U)
#define SYST_CSR            (*(volatile uint32_t *)0xE000E010U)
#define SYST_RVR            (*(volatile uint32_t *)0xE000E014U)
#define SYST_CVR            (*(volatile uint32_t *)0xE000E018U)

/* 中断和主循环共享，因此必须使用 volatile。32 位读取在本平台上是原子的。 */
static volatile uint32_t g_milliseconds;
static uint32_t g_led_is_on;

static void clock_init(void)
{
    RCC_CR |= (1U << 0);                 /* HSION：启用 HSI。 */
    while ((RCC_CR & (1U << 1)) == 0U) { /* HSIRDY：等待振荡器就绪。 */
    }

    /* SW = 00：系统时钟选 HSI；HPRE = 0000：AHB 不分频。 */
    RCC_CFGR &= ~((3U << 0) | (15U << 4));
    while ((RCC_CFGR & (3U << 2)) != 0U) { /* SWS：确认切换完成。 */
    }
}

static void led_init(void)
{
    RCC_APB2ENR |= (1U << 4); /* IOPCEN：开启 GPIOC 时钟。 */
    (void)RCC_APB2ENR;       /* 读回，确保时钟使能写入完成。 */

    /* 先预置高电平，再开启输出，避免初始化时短暂点亮。 */
    GPIOC_BSRR = LED_PIN;
    GPIOC_CRH &= ~(0xFU << 20);
    GPIOC_CRH |= (0x2U << 20); /* PC13：推挽输出，2MHz。 */
    g_led_is_on = 0U;
}

static void led_toggle(void)
{
    g_led_is_on ^= 1U;
    if (g_led_is_on != 0U) {
        GPIOC_BRR = LED_PIN;  /* 低电平亮，适用于常见 PC13 板载 LED。 */
    } else {
        GPIOC_BSRR = LED_PIN; /* 高电平灭。 */
    }
}

static void systick_init(void)
{
    SYST_CSR = 0U;
    SYST_RVR = (CORE_CLOCK_HZ / 1000U) - 1U; /* 8MHz / 1000 - 1 = 7999。 */
    SYST_CVR = 0U;                          /* 清空当前计数和 COUNTFLAG。 */
    /* CLKSOURCE = HCLK，TICKINT = 1，ENABLE = 1。 */
    SYST_CSR = (1U << 2) | (1U << 1) | (1U << 0);
}

void SysTick_Handler(void)
{
    ++g_milliseconds; /* ISR 只计时，不在这里延时或执行复杂任务。 */
}

static uint32_t millis(void)
{
    return g_milliseconds;
}

int main(void)
{
    uint32_t last_toggle;

    SCB_VTOR = 0x08000000U; /* 使用本程序在 Flash 中的中断向量表。 */
    clock_init();
    led_init();
    systick_init();
    last_toggle = millis();

    while (1) {
        uint32_t now = millis();

        if (interval_elapsed(now, last_toggle, LED_INTERVAL_MS)) {
            last_toggle = now;
            led_toggle();
        }

        /* 没有阻塞延时：以后可以在这里增加按键扫描、串口处理等任务。 */
    }
}
