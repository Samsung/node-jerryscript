#include "v8jerry_flags.hpp"

#include <cstddef>
#include <cstring>

static Flag BuildFlag(const char* name, bool default_value) {
    Flag newFlag{ Flag::Type::BOOL, name };
    newFlag.u.bool_value = default_value;
    return newFlag;
}

static Flag BuildFlag(const char* name, int default_value) {
    Flag newFlag{ Flag::Type::INT, name };
    newFlag.u.int_value = default_value;
    return newFlag;
}

static struct Flag flagsStore[] = {
#define FLAG_EXPAND(TYPE, NAME, DEFAULT_VALUE) BuildFlag(#NAME, DEFAULT_VALUE),
FLAGS(FLAG_EXPAND)
#undef FLAG_EXPAND
};

#define IS_SPACE(chr) ((chr) == ' ' || (chr) == '\t')

static const char* CompareFlag(const char* expected, const char* name, bool allow_multiple) {
    while (*expected != '\0') {
        const char ch = *expected++;
        const char inCh = *name++;

        if (ch != inCh) {
            if (ch != '_' || inCh != '-') {
                return NULL;
            }
        }
    }

    if (*name == '\0' || *name == '=' || (allow_multiple && IS_SPACE(*name))) {
        return name;
    }

    return NULL;
}

const char* Flag::Update(const char* name, bool allow_multiple) {
    bool negate = false;

    if (strncmp("no-", name, 3) == 0) {
        negate = true;
        name += 3;
    }

    const size_t size = sizeof(flagsStore) / sizeof(flagsStore[0]);

    for (size_t idx = 0; idx < size; idx++) {

        const char* end = CompareFlag(flagsStore[idx].name, name, allow_multiple);

        if (end != NULL) {
            if (flagsStore[idx].type == Flag::Type::BOOL) {
                if (*end == '=') {
                    return NULL;
                }

                flagsStore[idx].u.bool_value = !negate;
                return end;
            }

            if (*end != '=' || negate) {
                return NULL;
            }

            end++;

            if (*end < '1' || *end > '9') {
                return NULL;
            }

            int result = 0;

            while (true) {
                result = result * 10 + static_cast<int>(*end++ - '0');

                if (*end < '0' || *end > '9') {
                    if (*end == '\0' || (allow_multiple && IS_SPACE(*name))) {
                        flagsStore[idx].u.int_value = result;
                        return end;
                    }
                    return NULL;
                }
            }
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
