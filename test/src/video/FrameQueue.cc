#include <gtest/gtest.h>
#include "FrameQueue.h"

class FrameQueueTestFixture : public testing::Test
{
public:
	void SetUp() override
    {
	ASSERT_EQ(cuInit(0, __CUDA_API_VERSION, nullptr), CUDA_SUCCESS);
	ASSERT_EQ(cuvidInit(0), CUDA_SUCCESS);
	ASSERT_EQ(cuDeviceGet(&device, 0), CUDA_SUCCESS);
	ASSERT_EQ(cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device), CUDA_SUCCESS);
	ASSERT_EQ(cuCtxPopCurrent(&context), CUDA_SUCCESS);
	ASSERT_EQ(cuvidCtxLockCreate(&lock, context), CUDA_SUCCESS);

	queue = new CUVIDFrameQueue(lock);
	}

    void TearDown() override
    {
	delete queue;
	ASSERT_EQ(cuvidCtxLockDestroy(lock), CUDA_SUCCESS);
	ASSERT_EQ(cuCtxDestroy(context), CUDA_SUCCESS);
	}

protected:
	FrameQueue *queue;

private:
	CUdevice device;
	CUcontext context;
	CUvideoctxlock lock;
};

TEST_F(FrameQueueTestFixture, testInit)
{
	queue->init(1920, 1080);
	ASSERT_TRUE(queue->isEmpty());
	ASSERT_FALSE(queue->isEndOfDecode());
}

TEST_F(FrameQueueTestFixture, testEmptyDequeue)
{
	char data2[1024] = {0};
	queue->init(1920, 1080);
	ASSERT_FALSE(queue->dequeue(data2));
}

TEST_F(FrameQueueTestFixture, testEndOfDecode)
{
	queue->init(1920, 1080);
	queue->endDecode();
	ASSERT_TRUE(queue->isEndOfDecode());
}

TEST_F(FrameQueueTestFixture, testEnqueue)
{
	CUVIDPARSERDISPINFO parameters = {.picture_index = 1};

	queue->init(1920, 1080);

	ASSERT_TRUE(queue->isEmpty());
	queue->enqueue(&parameters);
    ASSERT_FALSE(queue->isEmpty());
}

TEST_F(FrameQueueTestFixture, testEnqueueDequeue)
{
	CUVIDPARSERDISPINFO parameters = {.picture_index = 1};

	queue->init(1920, 1080);

	ASSERT_TRUE(queue->isEmpty());
	queue->enqueue(&parameters);
	ASSERT_FALSE(queue->isEmpty());
	queue->dequeue(&parameters);
	ASSERT_TRUE(queue->isEmpty());
	ASSERT_EQ(parameters.picture_index, 1);
}

TEST_F(FrameQueueTestFixture, testMultipleEnqueue)
{
	CUVIDPARSERDISPINFO parameters = {.picture_index = 0};

	queue->init(1920, 1080);

	for(unsigned int i = 0; i < queue->cnMaximumSize; i++, parameters.picture_index++)
    {
		queue->enqueue(&parameters);
    }

    ASSERT_FALSE(queue->isEmpty());

    for(unsigned int i = 0; i < queue->cnMaximumSize; i++)
    {
        queue->dequeue(&parameters);
        ASSERT_EQ(parameters.picture_index, i);
    }

    ASSERT_TRUE(queue->isEmpty());
}
