#include <iostream>
#include <string>
#include <vector>

class a {
public:
    a() {
        std::cout << "a constructor" << '\n';
    };
    ~a() {
        std::cout << "a destructor" << '\n';
    };
};

class b : public a {
public:
    b() {
        std::cout << "b constructor" << '\n';
    };
    ~b() {
        std::cout << "b destructor" << '\n';
    };
};

int main() {

    std::cout << "Test!!! Start" << '\n';
    b* obj = new b();
    delete obj;

    std::cout << "Test!!! End" << '\n';

    return 0;
}
