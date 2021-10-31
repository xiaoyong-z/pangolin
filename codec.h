template<typename K, typename V>
class Entry {
public:
    Entry(K key, V value): key_(std::move(key)), value_(std::move(value)){};
private:
    K key_;
    V value_;
    uint64_t expires_at_;

};