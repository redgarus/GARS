#include <iostream>

int test(int a[], size_t i) {
    return a[i];
}

int main() {
    int a[3];
    std::cout << test(a, 1) << '\n';
    
}
