#include "lib/variant.h"

#include <iostream>

int main() {
    TVariant<int, double> v = 5;
    Visit([](auto value) {
        std::cout << value << std::endl;
    }, v);
    v = 1.5;
    Visit([](auto value) {
        std::cout << value << std::endl;
    }, v);

    return 0;
}
