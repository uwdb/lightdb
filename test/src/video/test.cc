#include <gtest/gtest.h>
#include "dynlink_cuda.h"
#include "dynlink_nvcuvid.h"

class FrameQueueTestFixture : public testing::Test
{
public:
        virtual void SetUp()
	{
	ASSERT_EQ(cuInit(0, __CUDA_API_VERSION, nullptr), CUDA_SUCCESS);
	ASSERT_EQ(cuvidInit(0), CUDA_SUCCESS);
	ASSERT_EQ(cuDeviceGet(&device, 0), CUDA_SUCCESS);
	ASSERT_EQ(cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device), CUDA_SUCCESS);
	ASSERT_EQ(cuCtxPopCurrent(&context), CUDA_SUCCESS);
	ASSERT_EQ(cuvidCtxLockCreate(&lock, context), CUDA_SUCCESS);
	}

        virtual void TearDown()
	{
	ASSERT_EQ(cuvidCtxLockDestroy(lock), CUDA_SUCCESS);
	ASSERT_EQ(cuCtxDestroy(context), CUDA_SUCCESS);
	}

private:
	CUdevice device;
	CUcontext context;
	CUvideoctxlock lock;
};

TEST_F(FrameQueueTestFixture, test_RLE)
{

}
