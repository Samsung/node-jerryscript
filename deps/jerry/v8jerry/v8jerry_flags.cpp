#include "v8jerry_flags.hpp"

#include <cstddef>
#include <cstring>

static Flag BuildFlag(Flag::Type type, const char* name, bool default_value) {
    Flag newFlag{ type, name };
    newFlag.u.bool_value = default_value;
    return newFlag;
}

struct Flag flagsStore[] = {
#define FLAG_EXPAND(TYPE, NAME, DEFAULT_VALUE) BuildFlag(Flag::Type::TYPE, #NAME, DEFAULT_VALUE),
FLAGS(FLAG_EXPAND)
#undef FLAG_EXPAND
};

static bool CompareFlag(const char* expected, const char* name) {
    for (size_t idx = 0; expected[idx] != '\0' && name[idx] != '\0'; idx++) {
        const char ch = expected[idx];
        const char inCh = name[idx];

        /* Treat '-' and '_' as same characters. */
        if ((ch == '_' && inCh == '-')
            || (ch == '-' && inCh == '_')) {
            continue;
        }

        if (expected[idx] != name[idx]) {
            return false;
        }
    }

    return true;
}

Flag* Flag::Get(const char* name) {
    for (size_t idx = 0; idx < sizeof(flagsStore) / sizeof(flagsStore[0]); idx++) {
        if (CompareFlag(flagsStore[idx].name, name)) {
            return &flagsStore[idx];
        }
    }

    return NULL;
}

Flag* Flag::Get(FlagID id) {
    if (id < 0 || id >= MAX_FLAG_VALUE) {
        return NULL;
    }

    return &flagsStore[id];
}
