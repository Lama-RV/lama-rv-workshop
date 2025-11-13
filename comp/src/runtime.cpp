#include "runtime.h"
#include <stdint.h>
#include "instructions.h"

namespace lama {

static constexpr char const* chars = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'";
static char* de_hash(int64_t n) {
    static char buf[10 + 1] = {0, 0, 0, 0, 0, 0};
    char* p = (char*)BOX(0);
    p = &buf[10];

    *p-- = 0;

    while (n != 0) {
        *p-- = chars[n & 0b111111];
        n = n >> 6;
    }
    return ++p;
}

int64_t LtagHash(const char* s) {
    const char* p = s;
    int64_t h = 0, limit = 0;
    while (*p && limit++ < 10) {
        char const* q = chars;
        int64_t pos = 0;

        for (; *q && *q != *p; q++, pos++)
            ;

        if (*q)
            h = (h << 6) | pos;
        else
            LOG(FATAL) << std::format("tagHash: character not found: {}", *p);

        p++;
    }

    if (strncmp(s, de_hash(h), 10) != 0) {
        LOG(FATAL) << std::format("{} <-> {}", s, de_hash(h));
    }

    return BOX(h);
}

}  // namespace lama
