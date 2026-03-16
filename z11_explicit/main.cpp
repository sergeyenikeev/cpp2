#include <iostream>
#include <string>

class Meter {
private:
    int value;

public:
    // Конструктор с explicit
    explicit Meter(int v) : value(v) {}

    void print() const {
        std::cout << value << " meters" << std::endl;
    }
};

void showMeter(Meter m) {
    m.print();
}

int main() {
    Meter a(10);   // OK: явный вызов конструктора
    a.print();

    // Meter b = 20; // Ошибка! Неявное преобразование запрещено из-за explicit

    Meter b(20);   // OK
    b.print();

    // showMeter(30); // Ошибка! int не преобразуется неявно в Meter

    showMeter(Meter(30)); // OK: явное преобразование

    return 0;
}
