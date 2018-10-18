#ifndef LIGHTDB_YOLO_LIBRARY_H
#define LIGHTDB_YOLO_LIBRARY_H

#include "Functor.h"

extern "C" {
    #include <darknet.h>

    // Not exposed by darknet, but should be
    float **make_probs(network *net);
};


class YOLO: public lightdb::functor::unaryfunctor {
    class CPU: public lightdb::functor::unaryfunction
    {
    public:
        CPU();
        CPU(const std::string &configuration_path,
            const std::string &weights_path,
            const std::string &metadata_path,
            const float cutoff=0.001f,
            float threshold=0.5f,
            float hierarchical_threshold=0.5f,
            float nms=0.45f)
                : lightdb::functor::unaryfunction(lightdb::physical::DeviceType::CPU,
                                                  lightdb::Codec::boxes(),
                                                  true),
                  network_(load_network(const_cast<char*>(configuration_path.c_str()),
                                        const_cast<char*>(weights_path.c_str()), 0)),
                  metadata_(get_metadata(const_cast<char*>(metadata_path.c_str()))),
                  boxes_(make_boxes(network_)),
                  probabilities_(make_probs(network_)),
                  box_count_(num_boxes(network_)),
                  threshold_(threshold),
                  hierarchical_threshold_(hierarchical_threshold),
                  nms_(nms),
                  total_size_(0)
        { }

        ~CPU() override {
            //free_network(network_);
        }

        lightdb::shared_reference<lightdb::LightField> operator()(lightdb::LightField& field) override;

    private:
        void Allocate(const unsigned int height, const unsigned int width, const unsigned int channels) {
            if(total_size_ != channels * height * width) {
                frame_size_ = height * width;
                total_size_ = channels * frame_size_;
                rgb_.resize(total_size_);
                scaled_.resize(total_size_);
                planes_.resize(total_size_);
            }
        }

        network * const network_;
        const metadata metadata_;
        box* const boxes_;
        float ** const probabilities_;
        const int box_count_;
        const float threshold_, hierarchical_threshold_, nms_;

        unsigned int frame_size_;
        unsigned int total_size_;
        std::vector<unsigned char> rgb_;
        std::vector<unsigned char> planes_;
        std::vector<float> scaled_;
    };

public:
    YOLO() : lightdb::functor::unaryfunctor("YOLO", CPU()) { }
};

#endif // LIGHTDB_YOLO_LIBRARY_H
