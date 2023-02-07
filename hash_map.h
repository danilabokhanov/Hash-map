#include <functional>
#include <stdexcept>
#include <list>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
public:
    HashMap(Hash hash = Hash()): hash_(hash) {
        initial_memory(INITIAL_SIZE);
    }

    template<typename init_iterator>
    HashMap(init_iterator begin, init_iterator end, Hash hash = Hash()): hash_(hash) {
        initial_memory(INITIAL_SIZE);
        for (auto it = begin; it != end; it++) {
            insert(*it);
        }
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>>
            &initial_list, Hash hash = Hash()): hash_(hash) {
        initial_memory(INITIAL_SIZE);
        for (auto it = initial_list.begin(); it != initial_list.end(); it++) {
            insert(*it);
        }
    }

    HashMap (const HashMap& other):hash_(other.hash_) {
        if (storage_ != other.storage_) {
            clear_memory();
            initial_memory(other.capacity_);
            pairs_.clear();
            used_positions_.clear();

            for (size_t i = 0; i < capacity_; i++) {
                if (other.used_[i] == 1) {
                    create_pair(i, other.storage_[i] -> first, other.storage_[i] -> second);
                } else if (other.used_[i] == 2) {
                    current_workload_++;
                }
                used_[i] = other.used_[i];
            }
        }
    }

    HashMap& operator=(const HashMap& other) {
        if (storage_ != other.storage_) {
            clear_memory();
            initial_memory(other.capacity_);
            pairs_.clear();
            used_positions_.clear();

            for (size_t i = 0; i < capacity_; i++) {
                if (other.used_[i] == 1) {
                    create_pair(i, other.storage_[i] -> first, other.storage_[i] -> second);
                } else if (other.used_[i] == 2) {
                    current_workload_++;
                }
                used_[i] = other.used_[i];
            }
        }
        return *this;
    }

    ~HashMap() {
        delete []storage_;
        delete []used_;
        delete []mark_indexes_;
    }

    void insert(const std::pair<KeyType, ValueType>& item) {
        size_t index = find_position(item.first);
        if (used_[index] == 1) {
            return;
        }
        create_pair(index, item.first, item.second);
    }

    void erase(const KeyType& key) {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            return;
        }
        pairs_.erase(storage_[index]);
        used_positions_.erase(mark_indexes_[index]);
        used_[index] = 2;
        size_--;
    }
    ValueType& operator[](const KeyType& key) {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            if (create_pair(index, key)) {
                return storage_[find_position(key)] -> second;
            }
        }
        return storage_[index] -> second;
    };

    const ValueType& at(const KeyType& key) const {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            throw std::out_of_range ("The key doesn't exist");
        }
        return storage_[index] -> second;
    };

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void clear() {
        for (auto j : used_positions_) {
            used_[j] = 2;
        }
        size_ = 0;
        used_positions_.clear();
        pairs_.clear();
    }

    using iterator = typename std::list<std::pair<const KeyType, ValueType>>::iterator;
    using const_iterator = typename std::list<std::pair<const KeyType, ValueType>>::const_iterator;

    iterator begin() {
        return iterator(pairs_.begin());
    }
    iterator end() {
        return iterator(pairs_.end());
    }

    const_iterator begin() const {
        return const_iterator(pairs_.begin());
    }
    const_iterator end() const {
        return const_iterator(pairs_.end());
    }

    const_iterator find(const KeyType& key) const {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            return end();
        }
        return const_iterator(storage_[index]);
    }
    iterator find(const KeyType& key) {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            return end();
        }
        return iterator(storage_[index]);
    }

    Hash hash_function() const {
        return hash_;
    }
private:
    constexpr static const size_t INITIAL_SIZE = 16;
    constexpr static const size_t SHIFT_HASH_FACTORS[] = {239, 179, 191};
    constexpr static const size_t TOP_LOAD_FACTOR = 50;
    constexpr static const size_t MAX_PERCENT = 100;
    Hash hash_;
    std::list<std::pair<const KeyType, ValueType>> pairs_;
    std::list<size_t> used_positions_;
    typename std::list<std::pair<const KeyType, ValueType>>::iterator* storage_ = nullptr;
    typename std::list<size_t>::iterator* mark_indexes_ = nullptr;
    size_t size_;
    size_t current_workload_;
    size_t capacity_;
    char* used_ = nullptr;
    void initial_memory(size_t new_capacity) {
        storage_ = new typename  std::list<std::pair<const KeyType, ValueType>>
        ::iterator[new_capacity];
        size_ = 0;
        current_workload_ = 0;
        capacity_ = new_capacity;
        used_ = new char[new_capacity];
        mark_indexes_ = new typename  std::list<size_t>
        ::iterator[new_capacity];
        for (size_t i = 0; i < capacity_; i++) {
            used_[i] = 0;
        }
    }

    void clear_memory() {
        delete []storage_;
        delete []used_;
        delete []mark_indexes_;
    }

    size_t compute_shift_hash(size_t primary_hash) const {
        size_t res = 0;
        for (size_t rate : SHIFT_HASH_FACTORS) {
            res = (res * primary_hash + rate) % capacity_;
        }
        return res;
    }

    size_t find_position(const KeyType& key) const {
        size_t hash = hash_(key), shift_hash = compute_shift_hash(hash);
        size_t index = hash % capacity_;
        while (used_[index] == 2 || (used_[index] == 1 && !(storage_[index] -> first == key))) {
            index = (index + shift_hash) % capacity_;
        }
        return index;
    }

    bool create_pair(size_t index, const KeyType& key, const ValueType& value = ValueType()) {
        pairs_.emplace_back(key, value);
        storage_[index] = --pairs_.end();
        used_positions_.push_back(index);
        mark_indexes_[index] = --used_positions_.end();
        size_++;
        current_workload_++;
        used_[index] = 1;
        if (check_overload()) {
            rebuild(capacity_ * 2);
            return true;
        }
        return false;
    }

    bool check_overload() const {
        return MAX_PERCENT * current_workload_ > TOP_LOAD_FACTOR * capacity_;
    }

    void rebuild(size_t new_capacity) {
        typename std::list<std::pair<const KeyType, ValueType>>::iterator* new_storage =
                new typename std::list<std::pair<const KeyType, ValueType>>
                ::iterator[new_capacity];
        typename std::list<size_t>::iterator* new_marked_indexes = new typename  std::list<size_t>
        ::iterator[new_capacity];

        size_t new_size = 0;
        size_t new_current_workload = 0;
        char* new_used = new char [new_capacity];
        for (size_t i = 0; i < new_capacity; i++) {
            new_used[i] = 0;
        }
        std::list<std::pair<const KeyType, ValueType>> new_pairs;

        std::list<size_t> new_used_positions;
        std::swap(new_capacity, capacity_);
        std::swap(new_storage, storage_);
        std::swap(new_size, size_);
        std::swap(new_current_workload, current_workload_);
        std::swap(new_used, used_);
        std::swap(new_used_positions, used_positions_);
        std::swap(new_marked_indexes, mark_indexes_);
        std::swap(new_pairs, pairs_);
        for (size_t i = 0; i < new_capacity; i++) {
            if (new_used[i] == 1) {
                size_t index = find_position(new_storage[i] -> first);
                create_pair(index, new_storage[i] -> first, new_storage[i] -> second);
            }
        }
        delete []new_storage;
        delete []new_used;
        delete []new_marked_indexes;
    }
};
