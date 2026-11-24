#ifndef TIMING_H
#define TIMING_H

#include <stdbool.h>
#include <stdint.h>

/* 无符号减法可跨越一次 uint32_t 回绕；需在计数器转满一圈前检查。 */
static inline bool interval_elapsed(uint32_t now, uint32_t since,
                                    uint32_t interval_ms)
{
    return (uint32_t)(now - since) >= interval_ms;
}

#endif
