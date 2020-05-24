#include "FrameQueue.h"
#include "VideoLock.h"
#include "RequiresGPUTest.h"

class FrameQueueTestFixture : public RequiresGPUTest {
public:
    FrameQueueTestFixture ()
        : queue([this]() { return CUVIDFrameQueue(lock); })
    { }

protected:
    lightdb::lazy<CUVIDFrameQueue> queue;
};

TEST_F(FrameQueueTestFixture, testInit) {
  ASSERT_TRUE(queue->isEmpty());
  ASSERT_FALSE(queue->isEndOfDecode());
}

TEST_F(FrameQueueTestFixture, testEmptyDequeue) {
  ASSERT_TRUE(queue->dequeue() == nullptr);
}

TEST_F(FrameQueueTestFixture, testEndOfDecode) {
  queue->endDecode();
  ASSERT_TRUE(queue->isEndOfDecode());
  ASSERT_TRUE(queue->isComplete());
}

TEST_F(FrameQueueTestFixture, testEnqueue) {
  CUVIDPARSERDISPINFO parameters{};

  parameters.picture_index = 1;

  ASSERT_TRUE(queue->isEmpty());
  queue->enqueue(&parameters);
  ASSERT_FALSE(queue->isEmpty());
  ASSERT_FALSE(queue->isComplete());
}

TEST_F(FrameQueueTestFixture, testEnqueueDequeue) {
  CUVIDPARSERDISPINFO parameters{};

  parameters.picture_index = 1;

  ASSERT_TRUE(queue->isEmpty());
  queue->enqueue(&parameters);
  ASSERT_FALSE(queue->isEmpty());
  ASSERT_TRUE(queue->dequeue() != nullptr);
  ASSERT_TRUE(queue->isEmpty());
  ASSERT_EQ(parameters.picture_index, 1);
}

TEST_F(FrameQueueTestFixture, testMultipleEnqueue) {
  CUVIDPARSERDISPINFO parameters = {};

  for (unsigned int i = 0; i < FrameQueue::cnMaximumSize; i++, parameters.picture_index++) {
    queue->enqueue(&parameters);
  }

  ASSERT_FALSE(queue->isEmpty());

  for (unsigned int i = 0; i < FrameQueue::cnMaximumSize; i++) {
    auto frame = queue->dequeue();
    ASSERT_NE(nullptr, frame);
    ASSERT_EQ(frame->picture_index, i);
  }

  ASSERT_TRUE(queue->isEmpty());
}
