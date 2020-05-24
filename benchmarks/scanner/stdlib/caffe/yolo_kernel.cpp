#include "scanner/api/op.h"

namespace scanner {

class YoloKernel : public BatchedKernel, public VideoKernel {
 public:
  YoloKernel(const KernelConfig& config) {}

//  void net_config() override {
//  }

// private:
//  f32 scale_;
};

REGISTER_OP(Yolo);
//    .frame_input("facenet_input")
//    .frame_output("facenet_output");

REGISTER_KERNEL(Yolo, YoloKernel)
    .device(DeviceType::CPU)
    .num_devices(Kernel::UnlimitedDevices);

//REGISTER_KERNEL(Yolo, YoloKernel).device(DeviceType::GPU).num_devices(1);
}
