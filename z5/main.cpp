#include <iostream>
#include <unordered_map>
#include <list>

class LRUCache {
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}

    int get(int key) {
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return -1;
        }

        touch(it);
        return it->second.first;
    }

    void put(int key, int value) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            it->second.first = value;
            touch(it);
            return;
        }

        if (cache_.size() == capacity_) {
            int oldKey = order_.back();
            order_.pop_back();
            cache_.erase(oldKey);
        }

        order_.push_front(key);
        cache_[key] = {value, order_.begin()};
    }

private:
    using ListIt = std::list<int>::iterator;

    void touch(std::unordered_map<int, std::pair<int, ListIt>>::iterator it) {
        int key = it->first;
        order_.erase(it->second.second);
        order_.push_front(key);
        it->second.second = order_.begin();
    }

    size_t capacity_;
    std::list<int> order_;
    std::unordered_map<int, std::pair<int, ListIt>> cache_;
};

int main() {
    LRUCache cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    std::cout << cache.get(1) << "\n"; // 10
    cache.put(3, 30);                  // удалится key=2
    std::cout << cache.get(2) << "\n"; // -1
}