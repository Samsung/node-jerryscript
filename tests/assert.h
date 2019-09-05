#ifndef ASSERT_V8_H_
#define ASSERT_V8_H_

#include <iostream>

#define ASSERT_EQUAL(a, b) \
    std::cout << a << " == " << b << " : "; \
    if (a != b) { \
        std::cout << "FAILED (" <<  __FILE__ << ":" << __LINE__ << ")" << std::endl; \
    } else { \
        std::cout << "OK" << std::endl; \
    }

std::ostream& operator<<(std::ostream& os, const uint8_t data) {
    os << unsigned(data);
    return os;
}

#endif /* ASSERT_V8_H_ */
