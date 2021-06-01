#ifndef V8JERRY_FLAGS_HPP
#define V8JERRY_FLAGS_HPP

#define FLAGS(F) \
    F(BOOL, expose_gc, false) \
    F(BOOL, use_strict, false) \
    F(BOOL, abort_on_uncaught_exception, false) \
    F(BOOL, stress_compaction, false) \
    F(BOOL, harmony_weak_refs, false) \
    F(BOOL, debug_code, false) \
    F(INT, stack_size, 4096) \
    F(INT, gc_interval, 0)

struct Flag {
    enum Type {
        BOOL,
        INT,
    };

    enum FlagID {
    #define FLAG_ENUM(TYPE, NAME, DEFAULT) NAME,
    FLAGS(FLAG_ENUM)
    #undef FLAG_ENUM
    };

    Type type;
    const char* name;

    union {
        bool bool_value;
        int int_value;
    } u;

    static const char* Update(const char* name, bool allow_multiple);
    static Flag* Get(FlagID);
};

#endif /* V8JERRY_FLAGS_HPP */
