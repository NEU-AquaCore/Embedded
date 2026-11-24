#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "timing.h"

int main(void)
{
    assert(!interval_elapsed(0U, 0U, 500U));
    assert(!interval_elapsed(499U, 0U, 500U));
    assert(interval_elapsed(500U, 0U, 500U));
    assert(interval_elapsed(501U, 0U, 500U));
    assert(!interval_elapsed(1499U, 1000U, 500U));
    assert(interval_elapsed(1500U, 1000U, 500U));

    /* since = 2^32 - 200；now = 300 时恰好过去 500ms。 */
    assert(!interval_elapsed(299U, UINT32_MAX - 199U, 500U));
    assert(interval_elapsed(300U, UINT32_MAX - 199U, 500U));
    assert(interval_elapsed(301U, UINT32_MAX - 199U, 500U));
    assert(interval_elapsed(123U, 123U, 0U));

    puts("PASS: 10 timing checks, including uint32_t wraparound.");
    return 0;
}
