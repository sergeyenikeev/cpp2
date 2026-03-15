// 2. Проверить, является ли число простым

// Задача:
// Написать функцию, которая определяет, является ли число простым.

#include <iostream>

bool is_prime(int num) {
    if (num <= 1) {
        return false; // Числа меньше или равные 1 не являются простыми
    }
    for (int i = 2; i <= num / 2; i++) {
        if (num % i == 0) {
            return false; // Число делится на другое число, значит оно не простое
        }
    }
    return true; // Число является простым
}

int main() {
    std::cout << "Test!!! Start" << '\n';
    int number = 29; // Пример числа для проверки
    std::cout << "Enter a number: ";
    // std::cin >> number;

    if (is_prime(number)) {
        std::cout << number << " is a prime number." << std::endl;
    } else {
        std::cout << number << " is not a prime number." << std::endl;
    }

    std::cout << "Test!!! End" << '\n';
    return 0;
}