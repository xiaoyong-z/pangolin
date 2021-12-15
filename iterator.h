#ifndef ITERATOR_H
#define ITERATOR_H
template <typename T, typename K>
class Iterator {
public:
    virtual ~Iterator() {};
    virtual bool hasNext() = 0;
    virtual void next() = 0;
    virtual const T& get() = 0;
    virtual const T& find(const K& key) = 0;
};

#endif 