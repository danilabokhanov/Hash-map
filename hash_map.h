#include <functional>
#include <stdexcept>

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

    HashMap (const HashMap& other):hash_(other.hash_), size_(other.size_), capacity_(other.capacity_) {
        pairs_ = new std::pair<KeyType, ValueType> [other.capacity_];
        std::copy(other.pairs_, other.pairs_ + other.capacity_, pairs_);
        used_ = new uint8_t [other.capacity_];
        std::copy(other.used_, other.used_ + other.capacity_, used_);
    }

    HashMap (HashMap&& other):hash_(other.hash_), size_(other.size_), capacity_(other.capacity_),
    used_(other.used_), pairs_(other.pairs_) {
        other.used_ = nullptr;
        other.pairs_ = nullptr;
    }

    HashMap& operator=(const HashMap& other) {
        if (this == &other) {
            return *this;
        }
        clear_memory();
        initial_memory(other.capacity_);

        for (size_t i = 0; i < capacity_; i++) {
            if (other.used_[i] == 1) {
                check_overload();
                create_pair(i, other.pairs_[i].first, other.pairs_[i].second);
            }
        }
        return *this;
    }

    HashMap& operator=(HashMap&& other) {
        if (this == &other) {
            return *this;
        }
        clear_memory();
        std::swap(used_, other.used_);
        std::swap(pairs_, other.pairs_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(hash_, other.hash_);
        return *this;
    }

    ~HashMap() {
        delete []used_;
        delete []pairs_;
        used_ = nullptr;
        pairs_ = nullptr;
    }

    void insert(const std::pair<KeyType, ValueType>& item) {
        check_overload();
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
        delete_pair(index);
        check_insufficient_load();
    }

    ValueType& operator[](const KeyType& key) {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            if (check_overload()) {
                index = find_position(key);
            }
            create_pair(index, key, ValueType{});
        }
        return pairs_[index].second;
    };

    const ValueType& at(const KeyType& key) const {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            throw std::out_of_range ("The key doesn't exist");
        }
        return pairs_[index].second;
    };

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void clear() {
        clear_memory();
        initial_memory(INITIAL_SIZE);
    }

    Hash hash_function() const {
        return hash_;
    }

    class iterator {
    public:
        iterator(): ptr_pair_(nullptr), ptr_used_(nullptr), end_used_(nullptr) {}
        iterator(std::pair<KeyType, ValueType> *ptr_pair, uint8_t *ptr_used, uint8_t *end_used) :
        ptr_pair_((std::pair<const KeyType, ValueType> *)ptr_pair), ptr_used_(ptr_used), end_used_(end_used) {}

        std::pair<const KeyType, ValueType>& operator*() {
            return *ptr_pair_;
        }

        std::pair<const KeyType, ValueType>* operator->() {
            return ptr_pair_;
        }

        iterator& operator++() {
            ++ptr_pair_;
            while (*(++ptr_used_) != 1 && ptr_used_ != end_used_) {
                ++ptr_pair_;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator cur = *this;
            ++ptr_pair_;
            while (*(++ptr_used_) != 1 && ptr_used_ != end_used_) {
                ++ptr_pair_;
            }
            return cur;
        }

        friend bool operator==(const iterator &a, const iterator &b) {
            return a.ptr_used_ == b.ptr_used_ && a.ptr_pair_ == b.ptr_pair_;
        }

        friend bool operator!=(const iterator &a, const iterator &b) {
            return a.ptr_used_ != b.ptr_used_ || a.ptr_pair_ != b.ptr_pair_;
        }

    private:
        std::pair<const KeyType, ValueType> *ptr_pair_ = nullptr;
        uint8_t *ptr_used_ = nullptr, *end_used_ = nullptr;
    };

    class const_iterator {
    public:
        const_iterator(): ptr_pair_(nullptr), ptr_used_(nullptr), end_used_(nullptr) {}
        const_iterator(const std::pair<KeyType, ValueType> *ptr_pair, uint8_t *ptr_used, uint8_t *end_used) :
        ptr_pair_((const std::pair<const KeyType, ValueType>*)ptr_pair), ptr_used_(ptr_used), end_used_(end_used) {}

        const std::pair<const KeyType, ValueType>& operator*() {
            return *ptr_pair_;
        }

        const std::pair<const KeyType, ValueType>* operator->() {
            return ptr_pair_;
        }

        const_iterator& operator++() {
            ++ptr_pair_;
            while (*(++ptr_used_) != 1 && ptr_used_ != end_used_) {
                ++ptr_pair_;
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator cur = *this;
            ++ptr_pair_;
            while (*(++ptr_used_) != 1 && ptr_used_ != end_used_) {
                ++ptr_pair_;
            }
            return cur;
        }

        bool operator==(const const_iterator &other) {
            return ptr_used_ == other.ptr_used_ && ptr_pair_ == other.ptr_pair_;
        }

        bool operator!=(const const_iterator &other) {
            return ptr_used_ != other.ptr_used_ || ptr_pair_ != other.ptr_pair_;
        }

    private:
        const std::pair<const KeyType, ValueType> *ptr_pair_ = nullptr;
        uint8_t *ptr_used_ = nullptr, *end_used_ = nullptr;
    };

    iterator begin() {
        uint8_t* begin = used_;
        while (*(begin) != 1 && begin != used_ + capacity_) {
            ++begin;
        }
        return iterator(pairs_ + (begin - used_), begin, used_ + capacity_);
    }
    iterator end() {
        return iterator(pairs_ + capacity_, used_ + capacity_, used_ + capacity_);
    }

    const_iterator begin() const {
        uint8_t* begin = used_;
        while (*(begin) != 1 && begin != used_ + capacity_) {
            ++begin;
        }
        return const_iterator(pairs_ + (begin - used_), begin, used_ + capacity_);
    }
    const_iterator end() const {
        return const_iterator(pairs_ + capacity_, used_ + capacity_, used_ + capacity_);
    }

    const_iterator find(const KeyType& key) const {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            return end();
        }
        return const_iterator(pairs_ + index, used_ + index, used_ + capacity_);
    }

    iterator find(const KeyType& key) {
        size_t index = find_position(key);
        if (used_[index] != 1) {
            return end();
        }
        return iterator(pairs_ + index, used_ + index, used_ + capacity_);
    }

private:
    constexpr static const size_t INITIAL_SIZE = 2;
    constexpr static const size_t SHIFT_HASH_FACTORS[] = {239, 179, 191};
    constexpr static const size_t BOTTOM_LOAD_FACTOR = 25;
    constexpr static const size_t TOP_LOAD_FACTOR = 50;
    constexpr static const size_t MAX_PERCENT = 100;
    Hash hash_;
    std::pair<KeyType, ValueType>* pairs_ = nullptr;
    size_t size_;
    size_t capacity_;
    uint8_t * used_ = nullptr;
    void initial_memory(size_t new_capacity) {
        pairs_ = new std::pair<KeyType, ValueType> [new_capacity];
        size_ = 0;
        capacity_ = new_capacity;
        used_ = new uint8_t [new_capacity];
        for (size_t i = 0; i < capacity_; i++) {
            used_[i] = 0;
        }
    }

    void clear_memory() {
        delete []used_;
        delete []pairs_;
        used_ = nullptr;
        pairs_ = nullptr;
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
        size_t first_zero = capacity_;
        while (used_[index] == 2 || (used_[index] == 1 && (pairs_[index].first != key))) {
            if (first_zero == capacity_ && used_[index] == 2) {
                first_zero = index;
            }
            index = (index + shift_hash) % capacity_;
        }
        if (used_[index] == 1 || pairs_[index].first == key) {
            return index;
        }
        if (first_zero != capacity_) {
            return first_zero;
        }
        return index;
    }

    void create_pair(size_t index, const KeyType& key, const ValueType& value = ValueType()) {
        pairs_[index].first = key;
        pairs_[index].second = value;
        size_++;
        used_[index] = 1;
    }

    void delete_pair(size_t index) {
        pairs_[index] = {KeyType{}, ValueType{}};
        size_--;
        used_[index] = 2;
    }

    bool check_overload() {
        if (MAX_PERCENT * (size_ + 1) > TOP_LOAD_FACTOR * capacity_) {
            rebuild(capacity_ * 2);
            return true;
        }
        return false;
    }

    void check_insufficient_load() {
        if (size_ >= 2 * INITIAL_SIZE && MAX_PERCENT * size_ < BOTTOM_LOAD_FACTOR * capacity_) {
            rebuild(capacity_ / 2);
        }
    }

    void rebuild(size_t new_capacity) {
        size_t new_size = 0;
        auto new_used = new uint8_t [new_capacity];
        for (size_t i = 0; i < new_capacity; i++) {
            new_used[i] = 0;
        }
       auto new_pairs = new std::pair<KeyType, ValueType> [new_capacity];

        std::swap(new_capacity, capacity_);
        std::swap(new_size, size_);
        std::swap(new_used, used_);
        std::swap(new_pairs, pairs_);
        for (size_t i = 0; i < new_capacity; i++) {
            if (new_used[i] == 1) {
                size_t index = find_position(new_pairs[i].first);
                create_pair(index, new_pairs[i].first, new_pairs[i].second);
            }
        }
        delete []new_used;
        delete []new_pairs;
    }
};
