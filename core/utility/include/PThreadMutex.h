#ifndef LIGHTDB_PTHREADMUTEX_H
#define LIGHTDB_PTHREADMUTEX_H

struct PThreadMutex
{
public:
    PThreadMutex() : mutex(PTHREAD_MUTEX_INITIALIZER) {}
    PThreadMutex(const PThreadMutex&) = delete;
    PThreadMutex(const PThreadMutex&&) = delete;

    void lock() { pthread_mutex_lock(&mutex);   }
    void unlock() { pthread_mutex_unlock(&mutex); }
private:
    pthread_mutex_t mutex;
};

#endif //LIGHTDB_PTHREADMUTEX_H
