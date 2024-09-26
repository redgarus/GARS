#include "lib.hpp"

namespace LIB {

static void test2() {
    cout << "test2\n";
}

void test1() {
    cout << "test1\n";
    test2();
}

}