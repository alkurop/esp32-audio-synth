#pragma once
#include <cstdint>

#pragma once

#include <cstdint>
#include <cstddef>

namespace ui
{

    /**
     * Copy up to MENU_TRUNCATE_LEN characters from src → dst,
     * null-terminating dst.
     */
    static inline void truncateForMenu(const char *src, char *dst, size_t size)
    {
        size_t i = 0;
        for (; i < size && src[i]; ++i) {
            dst[i] = src[i];
        }
        dst[i] = '\0';
    }
}
