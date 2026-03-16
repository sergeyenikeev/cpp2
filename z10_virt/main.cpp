#include <iostream>

class Base
{
public:
    ~Base()
    // virtual ~Base()
    {
        std::cout << "Hello from ~Base()" << std::endl;
    }
};

class Derived : public Base
{
public:
    ~Derived()
    // virtual ~Derived()
    {
        // Здесь могла бы быть очистка ресурсов
        std::cout << "Hello from ~Derived()" << std::endl;
    }
};

int main() {
    Base *obj = new Derived();
    delete obj;
    return 0;
}
