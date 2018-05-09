#include "scanner/api/op.h"
#include "scanner/util/serialize.h"
#include "scanner/util/opencv.h"
#include "stdlib/caffe/caffe_kernel.h"
#include "darknet.h"
#include "opencv2/opencv.hpp"
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudafilters.hpp"
//#include "opencv2/gpu/gpu.hpp"
#include <stdexcept>

using namespace cv;

namespace scanner {

class FacenetKernel : public CaffeKernel {
 public:
  FacenetKernel(const KernelConfig& config)
    : CaffeKernel(get_caffe_config(config)) {}

  void net_config() override {
    int net_input_width = frame_info_.shape[1];
    int net_input_height = frame_info_.shape[2];

    const boost::shared_ptr<caffe::Blob<float>> input_blob{
        net_->blob_by_name("data")};
    input_blob->Reshape({input_blob->shape(0), input_blob->shape(1),
                         net_input_width, net_input_height});
  }

  KernelConfig get_caffe_config(const KernelConfig& config) {
    proto::FacenetArgs args;
    args.ParseFromArray(config.args.data(), config.args.size());
    scale_ = args.scale();

    KernelConfig new_config(config);
    std::string caffe_string;
    args.caffe_args().SerializeToString(&caffe_string);
    new_config.args = std::vector<u8>(caffe_string.begin(), caffe_string.end());
    return new_config;
  }

 private:
  f32 scale_;
};

REGISTER_OP(Facenet)
    .frame_input("facenet_input")
    .frame_output("facenet_output");

REGISTER_KERNEL(Facenet, FacenetKernel)
    .device(DeviceType::CPU)
    .num_devices(Kernel::UnlimitedDevices);

REGISTER_KERNEL(Facenet, FacenetKernel).device(DeviceType::GPU).num_devices(1);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

image make_darknet_image(const Mat &frame, const size_t width, const size_t height)
{
    int w=width, h=height, c = frame.channels();

    int i,j,k;
    image im = make_image(w, h, c);
    for(k = 0; k < c; ++k){
        for(j = 0; j < h; ++j){
            for(i = 0; i < w; ++i){
                int dst_index = i + w*j + w*h*k;
                auto p = cv::Point(i, j);
                auto rgb = frame.at<Vec2b>(p);
                im.data[dst_index] = (float)rgb[k]/255.;
            }
        }
    }
    return im;
}



class YoloKernel : public BatchedKernel, public VideoKernel {
    public:
        YoloKernel(const KernelConfig& config)
                : BatchedKernel(config),
                  network_(load_network("/app/darknet/cfg/tiny-yolo.cfg", "/app/darknet/tiny-yolo.weights", 0)),
                  metadata_(get_metadata("/app/darknet/cfg/coco.data")),
                  box_count_(num_boxes(network_)),
                  boxes_(make_boxes(network_)),
                  probabilities_(make_probs(network_)),
                  threshold_(0.5f),
                  hier_threshold_(0.5f),
                  nms_(0.45f),
                  index_(0u)
        { }

    struct network *network_;
    metadata metadata_;
    int box_count_;
    box *boxes_;
    float **probabilities_;
    float threshold_, hier_threshold_, nms_;
    size_t index_;
    constexpr static size_t interval_ = 10;

    std::vector<BoundingBox> results_;

    void execute_frame(
            const Frame *frame,
            const BatchedColumns& input_columns,
            BatchedColumns& output_columns) {

        if(index_++ % interval_ == 0)
        {
            auto mat = frame_to_mat(frame);
            auto image = make_darknet_image(mat, frame->width(), frame->height());
            network_detect(network_, image, threshold_, hier_threshold_, nms_, boxes_, probabilities_);
            results_.clear();
            for(auto i = 0u; i < box_count_; i++)
                for(auto j = 0u; j < metadata_.classes; j++)
                    if(probabilities_[i][j] > 0.001)
                    {
                        BoundingBox box;
                        box.set_x1(boxes_[i].x);
                        box.set_y1(boxes_[i].y);
                        box.set_x2(boxes_[i].x + boxes_[i].w);
                        box.set_y2(boxes_[i].y + boxes_[i].h);
                        box.set_score(probabilities_[i][j]);
                        results_.emplace_back(box);
                    }

            //free_image(image);
        }

        u8* buffer;
        size_t size;
        serialize_bbox_vector(results_, buffer, size);
        insert_element(output_columns[0], buffer, size);
    }

    void execute(const BatchedColumns& input_columns,
                 BatchedColumns& output_columns) override {
        auto &frame_column = input_columns[0];

        for (auto i = 0u; i < frame_column.size(); i++)
        {
            //check_frame(scanner::CPU_DEVICE, frame_column[i]);
            execute_frame(frame_column[i].as_const_frame(), input_columns, output_columns);
        }
    }

};

    REGISTER_OP(Yolo)
        .frame_input("frame")
        .output("bboxes");

    REGISTER_KERNEL(Yolo, YoloKernel)
    .device(DeviceType::CPU)
    .num_devices(Kernel::UnlimitedDevices);

//REGISTER_KERNEL(Yolo, YoloKernel).device(DeviceType::GPU).num_devices(1);



    class SplitKernel : public BatchedKernel, public VideoKernel {
    public:
        SplitKernel(const KernelConfig& config)
                : BatchedKernel(config)
        { }

        static constexpr size_t rows = 3, columns = 4;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {
            auto &frame_column = input_columns[0];

            if(output_columns.size() != rows * columns)
                throw std::runtime_error("Invalid output column size");

            Size tile_size{frame_column[0].as_const_frame()->width()/columns,
                           frame_column[0].as_const_frame()->height()/rows};

            for (auto i = 0u; i < num_rows(frame_column); i++)
            {
                check_frame(scanner::CPU_DEVICE, frame_column[i]);

                auto *frame = frame_column[i].as_const_frame();
                cv::Mat mat = scanner::frame_to_mat(frame);

                for(auto row = 0u; row < rows; row++)
                    for(auto column = 0u; column < columns; column++)
                    {
                        auto offset_x = column * tile_size.width,
                             offset_y = row * tile_size.height;
                        auto range_x = Range(offset_x, offset_x + tile_size.width),
                                range_y = Range(offset_y, offset_y + tile_size.height);
                        auto tile_mat = mat(range_y, range_x);

                        auto output_frame_info = FrameInfo(tile_size.height, tile_size.width, frame->channels(), frame->type);
                        auto *output_frame = new_frame(scanner::CPU_DEVICE, output_frame_info);
                        auto output_mat = scanner::frame_to_mat(output_frame);

                        tile_mat.copyTo(output_mat);
                        insert_frame(output_columns[row * columns + column], output_frame);

                        //printf("starts %d\n", output_columns.size());
                        //insert_frame(output_columns[row * columns + column], copy_frame(frame));
                        //printf("end\n");
                    }
            }
        }

/*        Frame* copy_frame(const Frame *frame)
        {
            printf("copying\n");
            const auto info = FrameInfo(frame->as_frame_info());
            auto *out_frame = new_frame(scanner::CPU_DEVICE, info);
            memcpy(out_frame->data, frame->data, frame->size());
            return out_frame;
        }*/
    };

    REGISTER_OP(Split)
    .frame_input("frame")
    .frame_output("frame_output1")
    .frame_output("frame_output2")
    .frame_output("frame_output3")
    .frame_output("frame_output4")
    .frame_output("frame_output5")
    .frame_output("frame_output6")
    .frame_output("frame_output7")
    .frame_output("frame_output8")
    .frame_output("frame_output9")
    .frame_output("frame_output10")
    .frame_output("frame_output11")
    .frame_output("frame_output12");

    REGISTER_KERNEL(Split, SplitKernel)
    .device(DeviceType::CPU)
    .num_devices(Kernel::UnlimitedDevices);

    class JoinKernel : public BatchedKernel, public VideoKernel {
    public:
        JoinKernel(const KernelConfig& config)
                : BatchedKernel(config)
        { }

        static constexpr size_t rows = 3, columns = 4;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto *first_tile = input_columns[0][0].as_const_frame();
            auto tile_width = first_tile->width(),
                 tile_height = first_tile->height();
            auto &frame_column = input_columns[0];
            //auto &frame_column2 = input_columns[1];

            for (auto i = 0u; i < num_rows(frame_column); i++)
            {
                auto output_frame_info = FrameInfo(tile_height*rows, tile_width*columns, first_tile->channels(), first_tile->type);
                auto *output_frame = new_frame(scanner::CPU_DEVICE, output_frame_info);
                auto output_mat = scanner::frame_to_mat(output_frame);

                for(auto row = 0u; row < rows; row++)
                    for(auto column = 0u; column < columns; column++)
                    {
                        auto element = input_columns[row * columns + column][i];
                        check_frame(scanner::CPU_DEVICE, element);
                        auto *frame = element.as_const_frame();
                        auto tile_mat = scanner::frame_to_mat(frame);

                        auto offset_x = column * tile_width,
                             offset_y = row * tile_height;
                        auto range_x = Range(offset_x, offset_x + tile_width),
                             range_y = Range(offset_y, offset_y + tile_height);
                        auto target = output_mat(range_y, range_x);
                        tile_mat.copyTo(target);
                    }

                insert_frame(output_columns[0], output_frame);
            }
        }
    };

    REGISTER_OP(Join)
    .frame_input("frame1")
    .frame_input("frame2")
    .frame_input("frame3")
    .frame_input("frame4")
    .frame_input("frame5")
    .frame_input("frame6")
    .frame_input("frame7")
    .frame_input("frame8")
    .frame_input("frame9")
    .frame_input("frame10")
    .frame_input("frame11")
    .frame_input("frame12")
    .frame_output("frame_output");

    REGISTER_KERNEL(Join, JoinKernel)
    .device(DeviceType::CPU)
    .num_devices(Kernel::UnlimitedDevices);


    // Union Venice, Figure 14(c), wait maybe not?
    class TeeKernel : public BatchedKernel, public VideoKernel {
    public:
        TeeKernel(const KernelConfig& config)
                : BatchedKernel(config)
        { }

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();

            for (auto i = 0u; i < num_rows(frame_column); i++)
                for(auto column = 0u; column < output_columns.size(); column++)
                {
                    auto output_frame_info = FrameInfo(height, width, frame->channels(), frame->type);
                    auto *output_frame = new_frame(scanner::CPU_DEVICE, output_frame_info);
                    insert_frame(output_columns[column], output_frame);
                }
        }
    };

    REGISTER_OP(Tee)
    .frame_input("frame")
    .frame_output("frame_output1")
    .frame_output("frame_output2");

    REGISTER_KERNEL(Tee, TeeKernel)
    .device(DeviceType::CPU)
    .num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(Tee, TeeKernel).device(DeviceType::GPU).num_devices(1);



    // Union Venice, Figure 14(c)
    class OverlayKernel : public BatchedKernel, public VideoKernel {
    public:
        OverlayKernel(const KernelConfig& config)
                : BatchedKernel(config), device_(config.devices[0])
        { }

        DeviceHandle device_;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();
            //check_frame(scanner::GPU_DEVICE, frame_column[0]);

            for (auto i = 0u; i < num_rows(frame_column); i++)
                {
                    auto output_frame_info = FrameInfo(height, width, frame->channels(), frame->type);
                    auto *output_frame = new_frame(device_, output_frame_info);

                    auto mat = scanner::frame_to_gpu_mat(frame);
                    auto output_mat = scanner::frame_to_gpu_mat(output_frame);

                    mat.copyTo(output_mat);
                    insert_frame(output_columns[0], output_frame);
                }
        }
    };

    REGISTER_OP(Overlay)
    .frame_input("frame1")
    .frame_input("frame2")
    .frame_output("frame_output");

    //REGISTER_KERNEL(Overlay, OverlayKernel)
    //.device(DeviceType::CPU)
    //.num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(Overlay, OverlayKernel).device(DeviceType::GPU).num_devices(1);


    // Union Watermark, Figure 14(c)
    class OverlayWatermarkKernel : public BatchedKernel, public VideoKernel {
    public:
        OverlayWatermarkKernel(const KernelConfig& config)
                : BatchedKernel(config), device_(config.devices[0])
        { }

        DeviceHandle device_;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();
            //check_frame(scanner::GPU_DEVICE, frame_column[0]);

            for (auto i = 0u; i < num_rows(frame_column); i++)
            {
                auto output_frame_info = FrameInfo(height, width, frame->channels(), frame->type);
                auto *output_frame = new_frame(device_, output_frame_info);

                auto mat = scanner::frame_to_gpu_mat(frame);
                auto output_mat = scanner::frame_to_gpu_mat(output_frame);

                mat.copyTo(output_mat);
                insert_frame(output_columns[0], output_frame);
            }
        }
    };

    REGISTER_OP(OverlayWatermark)
    .frame_input("frame1")
    .frame_input("frame2")
    .frame_output("frame_output");

    //REGISTER_KERNEL(Overlay, OverlayKernel)
    //.device(DeviceType::CPU)
    //.num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(OverlayWatermark, OverlayWatermarkKernel).device(DeviceType::GPU).num_devices(1);


    // Union Rotated, Figure 14(c)
    class OverlayRotatedKernel : public BatchedKernel, public VideoKernel {
    public:
        OverlayRotatedKernel(const KernelConfig& config)
                : BatchedKernel(config), device_(config.devices[0])
        { }

        DeviceHandle device_;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();

            for (auto i = 0u; i < num_rows(frame_column); i++)
            {
                auto output_frame_info = FrameInfo(height, width, frame->channels(), frame->type);
                auto *output_frame = new_frame(device_, output_frame_info);

                auto mat = scanner::frame_to_gpu_mat(frame);
                auto output_mat = scanner::frame_to_gpu_mat(output_frame);

                auto left = mat(Range(0, height), Range(0, width/2));
                auto right = mat(Range(0, height), Range(width/2, width));

                mat.copyTo(output_mat);
                insert_frame(output_columns[0], output_frame);
            }
        }
    };

    REGISTER_OP(OverlayRotated)
    .frame_input("frame")
    .frame_output("frame_output");

    //REGISTER_KERNEL(Overlay, OverlayKernel)
    //.device(DeviceType::CPU)
    //.num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(OverlayRotated, OverlayRotatedKernel).device(DeviceType::GPU).num_devices(1);



    // Map grayscale, Figure 14(c)
    class GrayscaleKernel : public BatchedKernel, public VideoKernel {
    public:
        GrayscaleKernel(const KernelConfig& config)
                : BatchedKernel(config), device_(config.devices[0])
        { }

        DeviceHandle device_;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();

            for (auto i = 0u; i < num_rows(frame_column); i++)
            {
                auto output_frame_info = FrameInfo(height, width, frame->channels(), frame->type);
                auto *output_frame = new_frame(device_, output_frame_info);

                auto mat = scanner::frame_to_gpu_mat(frame);
                auto output_mat = scanner::frame_to_gpu_mat(output_frame);

               // mat.copyTo(output_mat);
                cvc::cvtColor(mat, output_mat, CV_BGR2GRAY);

                insert_frame(output_columns[0], output_frame);
            }
        }
    };

    REGISTER_OP(Grayscale)
    .frame_input("frame")
    .frame_output("frame_output");

    //REGISTER_KERNEL(Overlay, OverlayKernel)
    //.device(DeviceType::CPU)
    //.num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(Grayscale, GrayscaleKernel).device(DeviceType::GPU).num_devices(1);


    // Select pi/2 to pi, Figure 14(a)
    class SelectRightHalfKernel : public BatchedKernel, public VideoKernel {
    public:
        SelectRightHalfKernel(const KernelConfig& config)
                : BatchedKernel(config), device_(config.devices[0])
        { }

        DeviceHandle device_;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();

            for (auto i = 0u; i < num_rows(frame_column); i++)
            {
                auto output_frame_info = FrameInfo(height, width/2, frame->channels(), frame->type);
                auto *output_frame = new_frame(device_, output_frame_info);

                auto mat = scanner::frame_to_gpu_mat(frame);
                auto output_mat = scanner::frame_to_gpu_mat(output_frame);

                auto right = mat(Range(0, height), Range(width/2, width));

                right.copyTo(output_mat);
                insert_frame(output_columns[0], output_frame);
            }
        }
    };

    REGISTER_OP(SelectRightHalf)
    .frame_input("frame")
    .frame_output("frame_output");

    //REGISTER_KERNEL(Overlay, OverlayKernel)
    //.device(DeviceType::CPU)
    //.num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(SelectRightHalf, SelectRightHalfKernel).device(DeviceType::GPU).num_devices(1);


    // Select theta pi/2 to pi & phi pi/4 to pi/2, Figure 14(a)
    class SelectBottomRightCornerKernel : public BatchedKernel, public VideoKernel {
    public:
        SelectBottomRightCornerKernel(const KernelConfig& config)
                : BatchedKernel(config), device_(config.devices[0])
        { }

        DeviceHandle device_;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();

            for (auto i = 0u; i < num_rows(frame_column); i++)
            {
                auto output_frame_info = FrameInfo(height/2, width/2, frame->channels(), frame->type);
                auto *output_frame = new_frame(device_, output_frame_info);

                auto mat = scanner::frame_to_gpu_mat(frame);
                auto output_mat = scanner::frame_to_gpu_mat(output_frame);

                auto bottomright = mat(Range(height/2, height), Range(width/2, width));

                bottomright.copyTo(output_mat);
                insert_frame(output_columns[0], output_frame);
            }
        }
    };

    REGISTER_OP(SelectBottomRightCorner)
    .frame_input("frame")
    .frame_output("frame_output");

    //REGISTER_KERNEL(Overlay, OverlayKernel)
    //.device(DeviceType::CPU)
    //.num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(SelectBottomRightCorner, SelectBottomRightCornerKernel).device(DeviceType::GPU).num_devices(1);

    // Select t \in [1.5, 3.5], Figure 14(a)
    class IdentityKernel : public BatchedKernel, public VideoKernel {
    public:
        IdentityKernel(const KernelConfig& config)
                : BatchedKernel(config), device_(config.devices[0])
        { }

        DeviceHandle device_;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();

            for (auto i = 0u; i < num_rows(frame_column); i++)
            {
                auto output_frame_info = FrameInfo(height, width, frame->channels(), frame->type);
                auto *output_frame = new_frame(device_, output_frame_info);

                auto mat = scanner::frame_to_gpu_mat(frame);
                auto output_mat = scanner::frame_to_gpu_mat(output_frame);

                mat.copyTo(output_mat);
                insert_frame(output_columns[0], output_frame);
            }
        }
    };

    REGISTER_OP(identity)
    .frame_input("frame")
    .frame_output("frame_output");

    //REGISTER_KERNEL(Overlay, OverlayKernel)
    //.device(DeviceType::CPU)
    //.num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(identity, IdentityKernel).device(DeviceType::GPU).num_devices(1);


/*
    // Select t \in [1.5, 3.5], Figure 14(a)
    class SelectTemporalKernel : public BatchedKernel, public VideoKernel {
    public:
        SelectTemporalKernel(const KernelConfig& config)
                : BatchedKernel(config), device_(config.devices[0]), index_(0u)
        { }

        DeviceHandle device_;
        size_t index_;

        void execute(const BatchedColumns& input_columns,
                     BatchedColumns& output_columns) override {

            auto &frame_column = input_columns[0];
            auto *frame = frame_column[0].as_const_frame();
            auto width = frame->width(), height = frame->height();

            index_ += num_rows(frame_column);
            if(index_ > 45)
                for (auto i = 0u; i < num_rows(frame_column); i++)
                {
                    auto output_frame_info = FrameInfo(height/2, width/2, frame->channels(), frame->type);
                    auto *output_frame = new_frame(device_, output_frame_info);

                    auto mat = scanner::frame_to_gpu_mat(frame);
                    auto output_mat = scanner::frame_to_gpu_mat(output_frame);

                    auto bottomright = mat(Range(height/2, height), Range(width/2, width));

                    bottomright.copyTo(output_mat);
                    insert_frame(output_columns[0], output_frame);
                }
            else
                {
                    auto output_frame_info = FrameInfo(64, 64, frame->channels(), frame->type);
                    auto *output_frame = new_frame(device_, output_frame_info);
                    insert_frame(output_columns[0], output_frame);
                }
        }
    };

    REGISTER_OP(SelectTemporal)
    .frame_input("frame")
    .frame_output("frame_output");

    //REGISTER_KERNEL(Overlay, OverlayKernel)
    //.device(DeviceType::CPU)
    //.num_devices(Kernel::UnlimitedDevices);

    REGISTER_KERNEL(SelectTemporal, SelectTemporalKernel).device(DeviceType::GPU).num_devices(1);
*/
}
