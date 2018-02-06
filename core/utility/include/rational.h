#ifndef LIGHTDB_RATIONAL_H
#define LIGHTDB_RATIONAL_H

#include <boost/rational.hpp>

// Alias for boost:rational

namespace lightdb {
    using rational = boost::rational<unsigned int>;
}

#endif //LIGHTDB_RATIONAL_H
