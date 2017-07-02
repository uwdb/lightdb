#include "FrameQueue.h"
#include <gtest/gtest.h>
#include <GPUContext.h>
#include <DecoderLock.h>

class FrameQueueTestFixture : public testing::Test {
public:
    FrameQueueTestFixture () :
            context(0),
            lock(context),
            queue(lock.get())
    { }

protected:
    GPUContext context;
    DecoderLock lock;
    CUVIDFrameQueue queue;
};

TEST_F(FrameQueueTestFixture, testInit) {
  ASSERT_TRUE(queue.isEmpty());
  ASSERT_FALSE(queue.isEndOfDecode());
}

TEST_F(FrameQueueTestFixture, testEmptyDequeue) {
  char data2[1024] = {0};
  ASSERT_FALSE(queue.dequeue(data2));
}

TEST_F(FrameQueueTestFixture, testEndOfDecode) {
  queue.endDecode();
  ASSERT_TRUE(queue.isEndOfDecode());
}

TEST_F(FrameQueueTestFixture, testEnqueue) {
  CUVIDPARSERDISPINFO parameters = {.picture_index = 1};

  ASSERT_TRUE(queue.isEmpty());
  queue.enqueue(&parameters);
  ASSERT_FALSE(queue.isEmpty());
}

TEST_F(FrameQueueTestFixture, testEnqueueDequeue) {
  CUVIDPARSERDISPINFO parameters = {.picture_index = 1};

  ASSERT_TRUE(queue.isEmpty());
  queue.enqueue(&parameters);
  ASSERT_FALSE(queue.isEmpty());
  queue.dequeue(&parameters);
  ASSERT_TRUE(queue.isEmpty());
  ASSERT_EQ(parameters.picture_index, 1);
}

TEST_F(FrameQueueTestFixture, testMultipleEnqueue) {
  CUVIDPARSERDISPINFO parameters = {.picture_index = 0};

  for (unsigned int i = 0; i < queue.cnMaximumSize; i++, parameters.picture_index++) {
    queue.enqueue(&parameters);
  }

  ASSERT_FALSE(queue.isEmpty());

  for (unsigned int i = 0; i < queue.cnMaximumSize; i++) {
    queue.dequeue(&parameters);
    ASSERT_EQ(parameters.picture_index, i);
  }

  ASSERT_TRUE(queue.isEmpty());
}
