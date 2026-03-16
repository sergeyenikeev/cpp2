#include <iostream>

template <typename T>
class SharedPtr {
public:
    SharedPtr() : ptr_(nullptr), count_(nullptr) {}

    explicit SharedPtr(T* ptr) : ptr_(ptr), count_(new size_t(1)) {}

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), count_(other.count_) {
        inc();
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            count_ = other.count_;
            inc();
        }
        return *this;
    }

    ~SharedPtr() {
        release();
    }

    T* get() const {
        return ptr_;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    size_t use_count() const {
        return count_ ? *count_ : 0;
    }

private:
    void inc() {
        if (count_) {
            ++(*count_);
        }
    }

    void release() {
        if (count_) {
            --(*count_);
            if (*count_ == 0) {
                delete ptr_;
                delete count_;
            }
        }
        ptr_ = nullptr;
        count_ = nullptr;
    }

    T* ptr_;
    size_t* count_;
};

struct Test {
    int value = 42;
};

int main() {
    SharedPtr<Test> p1(new Test());
    {
        SharedPtr<Test> p2 = p1;
        std::cout << p1.use_count() << "\n"; // 2
        std::cout << p2->value << "\n";      // 42
    }
    std::cout << p1.use_count() << "\n";     // 1
}