#include <iostream>

template <typename Typson>
Typson test(Typson l, Typson r) { return l + r; }

int main() {
    printf("%i", test<int>(1,2));

    return 0;
}
