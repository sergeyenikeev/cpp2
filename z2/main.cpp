#include <iostream>
#include <string>

int main() {
//    Дана строка. Нужно развернуть её без использования std::reverse.
    std::cout << "Test!!! Start" << '\n';
    std::string str = "String for reverse";
    // std::string reversed_str;
    for (size_t i = 0; i < str.length() / 2; i++)
    {
        std::swap(str[i], str[str.length() - 1 - i]);
    }
    std::cout << str << std::endl;
    return 0;
}