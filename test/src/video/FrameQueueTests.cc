#include "FrameQueue.h"
#include <gtest/gtest.h>
#include <GPUContext.h>
#include <VideoLock.h>

class FrameQueueTestFixture : public testing::Test {
public:
    FrameQueueTestFixture () :
            context(0),
            lock(context),
            queue(lock.get())
    { }

protected:
    GPUContext context;
    VideoLock lock;
    CUVIDFrameQueue queue;
};

TEST_F(FrameQueueTestFixture, testInit) {
  ASSERT_TRUE(queue.isEmpty());
  ASSERT_FALSE(queue.isEndOfDecode());
}

TEST_F(FrameQueueTestFixture, testEmptyDequeue) {
  ASSERT_TRUE(queue.dequeue() == nullptr);
}

TEST_F(FrameQueueTestFixture, testEndOfDecode) {
  queue.endDecode();
  ASSERT_TRUE(queue.isEndOfDecode());
  ASSERT_TRUE(queue.isComplete());
}

TEST_F(FrameQueueTestFixture, testEnqueue) {
  CUVIDPARSERDISPINFO parameters = {.picture_index = 1};

  ASSERT_TRUE(queue.isEmpty());
  queue.enqueue(&parameters);
  ASSERT_FALSE(queue.isEmpty());
  ASSERT_FALSE(queue.isComplete());
}

TEST_F(FrameQueueTestFixture, testEnqueueDequeue) {
  CUVIDPARSERDISPINFO parameters = {.picture_index = 1};

  ASSERT_TRUE(queue.isEmpty());
  queue.enqueue(&parameters);
  ASSERT_FALSE(queue.isEmpty());
  ASSERT_TRUE(queue.dequeue() != nullptr);
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
    auto frame = queue.dequeue();
    ASSERT_NE(nullptr, frame);
    ASSERT_EQ(frame->picture_index, i);
  }

  ASSERT_TRUE(queue.isEmpty());
}
