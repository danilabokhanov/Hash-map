#include "hash_map.h"
#include <iostream>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <algorithm>

std::mt19937 rnd(std::chrono::steady_clock::now().time_since_epoch().count());

int get(int l, int r) {
    return rnd() % (r - l + 1) + l;
}

const int MIN_KEY = -100, MAX_KEY = 100, MIN_VALUE = -100, MAX_VALUE = 100, cnt = 2, N = 3, TESTS = 1e5;
const std::vector<std::string> operation = {"insert", "[]change", "[]", "erase", "find",
                                            "=", "clear"};

void output(std::vector<size_t> &ind, std::vector<std::string>& tp, std::vector<std::vector<int>>& lst) {
    std::cout << "WA\n";
    std::cout << ind.size() << " " << cnt << "\n";
    for (int i = 0; i < ind.size(); i++) {
        std::cout << "index: " << ind[i] << "\n";
        std::cout << tp[i] << " ";
        for (auto j : lst[i]) {
            std::cout << j << " ";
        }
        std::cout << "\n";
    }
    exit(0);
}

bool cmp_elements(std::unordered_map<int, int>& a, HashMap<int, int>& b) {
    std::vector<std::pair<int, int>> x, y;
    for (auto [key, val] : a) {
        x.push_back({key, val});
    }
    for (auto [key, val] : b) {
        y.push_back({key, val});
    }
    sort(x.begin(), x.end());
    sort(y.begin(), y.end());
    return x == y;
}

bool cmp_size(std::unordered_map<int, int>& a, HashMap<int, int>& b) {
    return a.size() == b.size();
}

bool cmp_pair(std::unordered_map<int, int>& a, HashMap<int, int>& b, int key) {
    auto it = a.find(key);
    auto it2 = b.find(key);
    if (it == a.end() && it2 == b.end()) {
        return true;
    }
    if (it == a.end() || it2 == b.end()) {
        return false;
    }
    return it -> second == it2 -> second;
}

int main() {
    for (int test = 0; test < TESTS; test++) {
        std::vector<size_t> ind;
        std::vector<std::string> tp;
        std::vector<std::vector<int>> lst;
        std::vector<std::unordered_map<int, int>> mp(cnt);
        std::vector<HashMap<int, int>> hash_map(cnt);
        for (int iter = 0; iter < N; iter++) {
            size_t index = get(0, cnt - 1);
            ind.push_back(index);
            std::string request = operation[get(0, operation.size() - 1)];
            tp.push_back(request);

            std::unordered_map<int, int>& a = mp[index];
            HashMap<int, int>&b = hash_map[index];
            if (request.front() == 'i') {
                int key = get(MIN_KEY, MAX_KEY), val = get(MIN_VALUE, MAX_VALUE);
                lst.push_back({key, val});
                a.insert({key, val});
                b.insert({key, val});
                if (!cmp_pair(a, b, key)) {
                    output(ind, tp, lst);
                }
            } else if (request[0] == '[' && request[2] == 'c') {
                int key = get(MIN_KEY, MAX_KEY), val = get(MIN_VALUE, MAX_VALUE);
                lst.push_back({key, val});
                a[key] = val;
                b[key] = val;
                if (!cmp_pair(a, b, key)) {
                    output(ind, tp, lst);
                }
            } else if (request[0] == '[') {
                int key = get(MIN_KEY, MAX_KEY);
                lst.push_back({key});
                a[key];
                b[key];
                if (!cmp_pair(a, b, key)) {
                    output(ind, tp, lst);
                }
            } else if (request[0] == 'e') {
                int key = get(MIN_KEY, MAX_KEY);
                lst.push_back({key});
                a.erase(key);
                b.erase(key);
                if (!cmp_pair(a, b, key)) {
                    output(ind, tp, lst);
                }
            } else if (request[0] == 'f'){
                int key = get(MIN_KEY, MAX_KEY);
                lst.push_back({key});
                auto it = a.find(key);
                auto it2 = b.find(key);
                if (!cmp_pair(a, b, key)) {
                    output(ind, tp, lst);
                }
            } else if (request[0] == '='){
                int id = get(0, cnt - 1);
                lst.push_back({id});
                a = mp[id];
                b = hash_map[id];
            } else if (request[0] == 'c') {
                lst.push_back({});
                a.clear();
                b.clear();
            }
            if (!cmp_size(a, b)) {
                output(ind, tp, lst);
            }
            if (!cmp_elements(a, b)) {
                output(ind, tp, lst);
            }
        }
        if (test % 1000 == 0) {
            std::cout << test << " " << "OK\n";
        }
    }
    return 0;
}
