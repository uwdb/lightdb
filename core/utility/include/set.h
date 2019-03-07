#ifndef LIGHTDB_SET_H
#define LIGHTDB_SET_H

#include <set>
#include <unordered_set>

namespace lightdb {

template<typename T>
class set: public std::set<T> {
public:
    T front() const { return *this->begin(); }
    T back() const { return *this->rbegin(); }
    T single() const {
        CHECK_EQ(this->size(), 1);
        return front();
    }
};

}; //namespace lightdb

#endif //LIGHTDB_SET_H
