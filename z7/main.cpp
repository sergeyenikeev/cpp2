#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// -----------------------------
// Демонстрационный пример:
// - несколько worker-потоков
// - общая очередь задач
// - атомарный счетчик обработанных задач
// - мьютекс для защиты очереди и результата
// -----------------------------

// Одна простая задача
struct Task {
    int id;
    int value;
};

// Глобальные / общие данные для примера
std::queue<Task> taskQueue;          // Общая очередь задач
std::mutex queueMutex;               // Защищает доступ к очереди
std::mutex printMutex;               // Защищает печать в консоль (чтобы сообщения не перемешивались)

std::vector<int> results;            // Общий контейнер результатов
std::mutex resultsMutex;             // Защищает доступ к results

std::atomic<int> processedCount{0};  // Атомарный счетчик обработанных задач
std::atomic<bool> stopFlag{false};   // Флаг остановки для потоков

// --------------------------------------------------------
// Функция "обработки" задачи
// В реальном проекте тут может быть работа с данными,
// вычисления, сетевой запрос и т.д.
// --------------------------------------------------------
int processTask(const Task& task) {
    // Имитируем полезную работу
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Для примера просто возводим value в квадрат
    return task.value * task.value;
}

// --------------------------------------------------------
// Функция worker-потока
// Каждый поток:
// 1. Пытается забрать задачу из общей очереди
// 2. Обрабатывает ее
// 3. Сохраняет результат
// 4. Увеличивает атомарный счетчик
// --------------------------------------------------------
void worker(int workerId) {
    while (!stopFlag.load()) {
        Task currentTask;
        bool hasTask = false;

        // -----------------------------
        // Критическая секция №1:
        // доступ к общей очереди задач
        // -----------------------------
        {
            std::lock_guard<std::mutex> lock(queueMutex);

            if (!taskQueue.empty()) {
                currentTask = taskQueue.front();
                taskQueue.pop();
                hasTask = true;
                {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "[Worker " << workerId
                          << "] Start task id=" << currentTask.id
                          << ", value=" << currentTask.value << '\n';
                }
                
            }
        }
        // lock_guard выходит из области видимости -> mutex автоматически освобождается

        if (hasTask) {
            // Обрабатываем задачу вне мьютекса
            // Это важно: нельзя держать mutex дольше, чем нужно
            int result = processTask(currentTask);

            // -----------------------------
            // Критическая секция №2:
            // доступ к общему вектору результатов
            // -----------------------------
            {
                std::lock_guard<std::mutex> lock(resultsMutex);
                results.push_back(result);

            }
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[Worker " << workerId
                        << "] save result task id=" << currentTask.id
                        << ", result=" << result << '\n';
            }

            // Атомарное увеличение счетчика
            // Несколько потоков могут безопасно делать ++processedCount
            ++processedCount;
        } else {
            // Если задач пока нет, немного подождем,
            // чтобы не крутить CPU в пустом цикле
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "[Worker " << workerId << "] finished task\n";
    }
}

int main() {
    // -----------------------------
    // Шаг 1. Наполняем очередь задачами
    // -----------------------------
    std::cout << "_Step 1" << '\n';
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (int i = 1; i <= 10; ++i) {
            taskQueue.push(Task{i, i + 10});
        }
    }

    // -----------------------------
    // Шаг 2. Создаем несколько потоков
    // -----------------------------
    std::cout << "_Step 2" << '\n';
    const int threadCount = 3;
    std::vector<std::thread> workers;

    for (int i = 0; i < threadCount; ++i) {
        workers.emplace_back(worker, i + 1);
    }

    // -----------------------------
    // Шаг 3. Ждем, пока все задачи будут обработаны
    // processedCount - атомарный, поэтому читать его безопасно
    // -----------------------------
    std::cout << "_Step 3" << '\n';
    while (processedCount.load() < 10) {
        std::cout << "[Main] finished tasks: "
                  << processedCount.load() << " in 10\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // -----------------------------
    // Шаг 4. Даем сигнал потокам остановиться
    // -----------------------------
    std::cout << "_Step 4" << '\n';
    stopFlag.store(true);

    // -----------------------------
    // Шаг 5. Ждем завершения всех потоков
    // -----------------------------
    std::cout << "_Step 5" << '\n';
    for (auto& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }

    // -----------------------------
    // Шаг 6. Печатаем итог
    // -----------------------------
    std::cout << "_Step 6" << '\n';
    std::cout << "\nItog:\n";
    std::cout << "All make: " << processedCount.load() << '\n';

    std::cout << "Result: ";
    {
        std::lock_guard<std::mutex> lock(resultsMutex);
        for (int value : results) {
            std::cout << value << ' ';
        }
    }
    std::cout << '\n';

    return 0;
}
