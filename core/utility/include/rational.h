#ifndef LIGHTDB_RATIONAL_H
#define LIGHTDB_RATIONAL_H

#include <boost/rational.hpp>

// Alias for boost:rational

namespace lightdb {
    //TODO change to std::ratio
    class rational: public boost::rational<unsigned int> {
    public:
        using boost::rational<unsigned int>::rational;

        explicit operator double() const { return numerator() / static_cast<double>(denominator()); }
    };
}

#endif //LIGHTDB_RATIONAL_H
