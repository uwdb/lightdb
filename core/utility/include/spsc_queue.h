#ifndef LIGHTDB_SPSC_QUEUE_H
#define LIGHTDB_SPSC_QUEUE_H

#include <boost/lockfree/spsc_queue.hpp>

namespace lightdb {
    template<typename T>
    using spsc_queue = boost::lockfree::spsc_queue<T>;
}; // namesapce lightdb

#endif //LIGHTDB_SPSC_QUEUE_H
