#include <atomic>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <sstream>

// ============================================================
// Более сложный пример многопоточности на C++
// ------------------------------------------------------------
// Идея:
//   - Есть простой ThreadPool
//   - В него можно добавлять задачи
//   - Несколько worker-потоков ждут задачи через condition_variable
//   - Общая очередь защищена mutex
//   - Статистика хранится в atomic
//   - Есть корректная остановка пула
// ============================================================

class ThreadPool {
public:
    ThreadPool(size_t threadCount)
        : stopRequested_(false),
          tasksSubmitted_(0),
          tasksCompleted_(0),
          activeWorkers_(0) {
        workers_.reserve(threadCount);

        for (size_t i = 0; i < threadCount; ++i) {
            workers_.emplace_back(&ThreadPool::workerLoop, this, i);
        }
    }

    // Запрещаем копирование, потому что класс владеет потоками и mutex/cv
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool() {
        shutdown();
    }

    // ------------------------------------------------------------
    // submit:
    // добавляет новую задачу в очередь
    // ------------------------------------------------------------
    void submit(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);

            // После начала остановки новые задачи принимать нельзя
            if (stopRequested_) {
                throw std::runtime_error("ThreadPool is stopping; cannot submit new tasks");
            }

            tasks_.push(std::move(task));
            ++tasksSubmitted_;
        }

        // Будим один ожидающий поток
        condition_.notify_one();
    }

    // ------------------------------------------------------------
    // shutdown:
    // корректно останавливает пул
    // ------------------------------------------------------------
    void shutdown() {
        bool expected = false;
        if (!stopRequested_.compare_exchange_strong(expected, true)) {
            // shutdown уже был вызван ранее
            return;
        }

        // Разбудим все потоки, чтобы они могли завершиться
        condition_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // ------------------------------------------------------------
    // Методы для чтения статистики
    // ------------------------------------------------------------
    size_t tasksSubmitted() const {
        return tasksSubmitted_.load();
    }

    size_t tasksCompleted() const {
        return tasksCompleted_.load();
    }

    size_t activeWorkers() const {
        return activeWorkers_.load();
    }

private:
    // Очередь задач
    std::queue<std::function<void()>> tasks_;

    // Потоки-воркеры
    std::vector<std::thread> workers_;

    // Синхронизация доступа к очереди
    std::mutex queueMutex_;
    std::condition_variable condition_;

    // Для красивого потокобезопасного логирования
    std::mutex coutMutex_;

    // Флаг остановки
    std::atomic<bool> stopRequested_;

    // Атомарная статистика
    std::atomic<size_t> tasksSubmitted_;
    std::atomic<size_t> tasksCompleted_;
    std::atomic<size_t> activeWorkers_;

private:
    // ------------------------------------------------------------
    // workerLoop:
    // основной цикл worker-потока
    // ------------------------------------------------------------
    void workerLoop(size_t workerId) {
        for (;;) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queueMutex_);

                // Ждем, пока:
                // 1) появится задача
                // ИЛИ
                // 2) поступит сигнал остановки
                //
                // Важно использовать predicate-версию wait,
                // чтобы корректно обрабатывать spurious wakeup.
                condition_.wait(lock, [this]() {
                    return stopRequested_.load() || !tasks_.empty();
                });

                // Если остановка запрошена и задач больше нет — выходим
                if (stopRequested_.load() && tasks_.empty()) {
                    log(workerId, "stopping: no more tasks");
                    return;
                }

                // Иначе забираем задачу из очереди
                task = std::move(tasks_.front());
                tasks_.pop();
            }

            // Считаем, сколько сейчас реально занятых worker-ов
            ++activeWorkers_;

            try {
                log(workerId, "started task");
                task();
                log(workerId, "finished task");
            } catch (const std::exception& ex) {
                log(workerId, std::string("task threw exception: ") + ex.what());
            } catch (...) {
                log(workerId, "task threw unknown exception");
            }

            --activeWorkers_;
            ++tasksCompleted_;
        }
    }

    // ------------------------------------------------------------
    // Потокобезопасный лог
    // ------------------------------------------------------------
    void log(size_t workerId, const std::string& message) {
        std::lock_guard<std::mutex> lock(coutMutex_);
        std::cout << "[worker " << workerId << "] " << message << '\n';
    }
};

// ============================================================
// Демонстрационная "бизнес-логика"
// ============================================================

// Имитация CPU/IO задачи
int heavyComputation(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    return x * x;
}

int main() {
    try {
        const size_t threadCount = 4;
        ThreadPool pool(threadCount);

        // Общий контейнер результатов
        std::vector<int> results;
        std::mutex resultsMutex;

        // Дополнительная атомарная статистика
        std::atomic<long long> totalSum{0};

        // --------------------------------------------------------
        // Кладем задачи в пул
        // Каждая задача:
        //   1) считает результат
        //   2) добавляет его в общий вектор под mutex
        //   3) обновляет общую сумму через atomic
        // --------------------------------------------------------
        for (int i = 1; i <= 12; ++i) {
            pool.submit([i, &results, &resultsMutex, &totalSum]() {
                int result = heavyComputation(i);

                // vector не потокобезопасен -> нужен mutex
                {
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    results.push_back(result);
                }

                // Для суммы используем атомарную операцию
                totalSum.fetch_add(result, std::memory_order_relaxed);
            });
        }

        // --------------------------------------------------------
        // В main-потоке периодически печатаем статистику
        // --------------------------------------------------------
        while (pool.tasksCompleted() < pool.tasksSubmitted()) {
            std::cout
                << "[main] submitted=" << pool.tasksSubmitted()
                << ", completed=" << pool.tasksCompleted()
                << ", activeWorkers=" << pool.activeWorkers()
                << ", totalSum=" << totalSum.load(std::memory_order_relaxed)
                << '\n';

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Явно завершаем пул
        pool.shutdown();

        // --------------------------------------------------------
        // Печать результата
        // --------------------------------------------------------
        {
            std::lock_guard<std::mutex> lock(resultsMutex);

            std::cout << "\nFinal results (" << results.size() << " items): ";
            for (int value : results) {
                std::cout << value << ' ';
            }
            std::cout << '\n';
        }

        std::cout << "Final totalSum = "
                  << totalSum.load(std::memory_order_relaxed) << '\n';
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
