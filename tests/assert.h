#ifndef ASSERT_V8_H_
#define ASSERT_V8_H_

#include <iostream>
#include <functional>
#include <type_traits>
#include <cstring>

namespace _TEST_LOG {

struct str_equal_to {
    bool operator()(const char* const &lhs, const char* const &rhs) const
    {
        return strcmp(lhs, rhs) == 0;
    }
};

template<typename T, typename OP>
static inline void _TEST_LOG(std::string file, int line, const char* arg_a_str, const T arg_a, const char* arg_b_str, const T arg_b) {
    size_t last_separator_pos = file.find_last_of("/");
    std::cout << file.substr(last_separator_pos + 1) << "(" << line << ") : ";
    OP op;
    if (!op(arg_a, arg_b)) {
        std::cout << "FAILED";
    } else {
        std::cout << "OK    ";
    }

    std::cout << "[" << arg_a_str << "] " << arg_a << " == " << arg_b << " [" << arg_b_str << "] " << std::endl;
}

} // namespace _TEST_LOG


#define ASSERT_EQUAL(a, b) do { \
    typedef typename std::remove_reference<decltype(a)>::type BASE_TYPE; \
    _TEST_LOG::_TEST_LOG<BASE_TYPE, std::equal_to<BASE_TYPE>>(__FILE__, __LINE__, #a, a, #b, b); \
} while(0)

#define ASSERT_NOT_EQUAL(a, b) do { \
    typedef typename std::remove_reference<decltype(a)>::type BASE_TYPE; \
    _TEST_LOG::_TEST_LOG<BASE_TYPE, std::not_equal_to<BASE_TYPE>>(__FILE__, __LINE__, #a, a, #b, b); \
} while(0)


#define ASSERT_STR_EQUAL(a, b) do { \
    _TEST_LOG::_TEST_LOG<const char*, _TEST_LOG::str_equal_to>(__FILE__, __LINE__, #a, (const char*)a, #b, (const char*)b); \
} while(0)

inline std::ostream& operator<<(std::ostream& os, const uint8_t data) {
    os << unsigned(data);
    return os;
}

#endif /* ASSERT_V8_H_ */
