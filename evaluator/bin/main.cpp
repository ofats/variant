#include "evaluator/evaluator.h"

#include <iostream>

int main() {
    std::string expr;
    std::cout << "> ";
    std::cout.flush();
    while (std::getline(std::cin, expr)) {
        if (expr.empty()) {
            break;
        }
        try {
            std::cout << evaler::eval(evaler::parse(expr)) << std::endl;
        } catch (std::runtime_error& e) {
#if defined(__gnu_linux__)
            std::cerr << "\033\[01;31m" << e.what() << "\033\[01;0m"
                      << std::endl;
#else
            std::cerr << e.what() << std::endl;
#endif
        }
        std::cout << "> ";
        std::cout.flush();
    }
    return 0;
}
