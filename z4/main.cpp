// 3. Найти максимальный элемент массива

// Задача:
// Дан массив целых чисел. Найти максимальный элемент.
#include <iostream>
#include <vector>

int findMax(const std::vector<int>& v) {
    if (v.empty()) {
        return 0; // Возвращаем 0 для пустого массива, можно также выбросить исключение
    }
    int maxelement = v[0];
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i] > maxelement) {
            maxelement = v[i];

        }
    }
    return maxelement;

}

int main() {
    std::cout << "Test!!! Start" << '\n';
    std::vector<int> arr = {3, 5, 7, 2, 8, 1, 9, 1, 4, 5};
    int maxElement = findMax(arr);
    std::cout << "Maximum element in the array: " << maxElement << std::endl;
    std::cout << "Test!!! End" << '\n';
    return 0;
}