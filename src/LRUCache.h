#include <unordered_map>
#include <list>

class LRUCache {
private:
    int capacity;
    std::unordered_map<int, std::pair<int, std::list<int>::iterator>> cache;
    std::list<int> lruList;

public:
    LRUCache(int cap) {
        capacity = cap;
    }

    int get(int key) {
        if (cache.find(key) == cache.end()) {
            return -1;  // Key doesn't exist in the cache
        }
        
        // Move the accessed key to the front of LRU list
        lruList.erase(cache[key].second);
        lruList.push_front(key);
        
        // Update the iterator in the cache map
        cache[key].second = lruList.begin();
        
        return cache[key].first;
    }

    void put(int key, int value) {
        if (cache.find(key) != cache.end()) {
            // Key already exists, move it to the front of LRU list and update the value
            lruList.erase(cache[key].second);
        }
        else if (cache.size() >= capacity) {
            // Cache is full, remove the least recently used key from both the list and the cache
            int lastKey = lruList.back();
            lruList.pop_back();
            cache.erase(lastKey);
        }
        
        // Insert the new key-value pair and add the key to the front of LRU list
        lruList.push_front(key);
        cache[key] = {value, lruList.begin()};
    }
};