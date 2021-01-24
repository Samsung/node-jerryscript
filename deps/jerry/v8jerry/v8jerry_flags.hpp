#ifndef V8JERRY_FLAGS_HPP
#define V8JERRY_FLAGS_HPP

#define FLAGS(F) \
    F(BOOL, expose_gc, false) \
    F(BOOL, abort_on_uncaught_exception, false)

struct Flag {
    enum Type {
        BOOL,
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
    } u;

    static Flag* Get(const char* name);
    static Flag* Get(FlagID);
};

#endif /* V8JERRY_FLAGS_HPP */
