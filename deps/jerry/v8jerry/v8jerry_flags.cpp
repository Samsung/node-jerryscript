#include "v8jerry_flags.hpp"

#include <cstddef>
#include <cstring>

static Flag BuildFlag(Flag::Type type, const char* name, bool default_value) {
    Flag newFlag{ type, name };
    newFlag.u.bool_value = default_value;
    return newFlag;
}

static struct Flag flagsStore[] = {
#define FLAG_EXPAND(TYPE, NAME, DEFAULT_VALUE) BuildFlag(Flag::Type::TYPE, #NAME, DEFAULT_VALUE),
FLAGS(FLAG_EXPAND)
#undef FLAG_EXPAND
};

static bool CompareFlag(const char* expected, const char* name) {
    size_t idx;

    for (idx = 0; expected[idx] != '\0'; idx++) {
        const char ch = expected[idx];
        const char inCh = name[idx];

        if (ch != inCh) {
            if (ch != '_' || inCh != '-') {
                return false;
            }
        }
    }

    return name[idx] == '\0';
}

Flag* Flag::Get(const char* name) {
    const size_t size = sizeof(flagsStore) / sizeof(flagsStore[0]);

    for (size_t idx = 0; idx < size; idx++) {
        if (CompareFlag(flagsStore[idx].name, name)) {
            return &flagsStore[idx];
        }
    }

    return NULL;
}

Flag* Flag::Get(FlagID id) {
    const size_t size = sizeof(flagsStore) / sizeof(flagsStore[0]);

    if (id < 0 || id >= size) {
        return NULL;
    }

    return &flagsStore[id];
}
