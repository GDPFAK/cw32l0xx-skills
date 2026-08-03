/*
 * System/sys_assert.c
 * SDK 断言失败回调（USE_FULL_ASSERT 开启时被引用）。
 */
#include <stdint.h>

void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    for (;;)
        ;
}