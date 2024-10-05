#include <iostream>
#include <vector>
#include <memory>

using namespace std;

class Inter {
public:
    virtual void test() = 0;
};

class Test: public Inter {
};

void Test::Inter::test() {
    cout << "Testik\n";
}


int main() {
    Test::Inter::test();

    
}
