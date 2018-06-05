#ifndef LIGHTDB_UNIONOPERATORS_H
#define LIGHTDB_UNIONOPERATORS_H

#include "PhysicalOperators.h"
#include "EncodeOperators.h"

namespace lightdb::physical {

class GPUUnion : public GPUOperator {
public:
    explicit GPUUnion(const LightFieldReference &logical,
                      std::vector<PhysicalLightFieldReference> &parents,
                      const Codec &codec,
                      const unsigned int rows, const unsigned int columns)
            : GPUOperator(logical, parents,
                          parents[0].downcast<GPUOperator>().gpu(),
                          [this, rows, columns]() {
                              return create_configuration(rows, columns); }),
              rows_(rows),
              columns_(columns),
              frames_([this] { return functional::make_union_iterator<GPUFrameReference>(
                      functional::transform<functional::flatmap_iterator<GPUFrameReference, PhysicalLightField::downcast_iterator<GPUDecodedFrameData>>>(iterators().begin(), iterators().end(), [](PhysicalLightField::iterator &it) { return functional::make_flatmap_iterator<GPUFrameReference>(it.downcast<GPUDecodedFrameData>()); })); })
    { }

    GPUUnion(const GPUUnion &) = delete;

    std::optional<physical::DataReference> read() override {
        if(!any_parent_eos()) {
            std::vector<GPUFrameReference> out;
            auto available_frames = std::max(frames_->available(), 1ul);

            for(auto i = 0u; i < available_frames; i++) {
                auto frames = frames_.value()++;
                auto unioned = GPUFrameReference::make<CudaFrame>(configuration().height, configuration().width, frames[0]->type());
                auto &cuda = unioned.downcast<CudaFrame>();

                for(auto column = 0u; column < columns_; column++)
                    for(auto row = 0u; row < rows_; row++)
                        cuda.copy(lock(), *frames[row * columns_ + column]->cuda(),
                                  configuration().height / rows_,
                                  configuration().width / columns_);

                out.emplace_back(GPUFrameReference{unioned});
            }

            return GPUDecodedFrameData{out};
        } else
            return {};
    }

private:
    Configuration create_configuration(const unsigned int rows, const unsigned int columns) {
        CHECK(!parents().empty());

        const auto &configuration = parents()[0].downcast<GPUOperator>().configuration();
        return Configuration{configuration.width * columns, configuration.height * rows, 0, 0,
                             configuration.bitrate, configuration.framerate};
    }

    const unsigned int rows_, columns_;
    lazy<functional::union_iterator<
            GPUFrameReference,
            std::vector<
                    functional::flatmap_iterator<GPUFrameReference,
                                                 PhysicalLightField::downcast_iterator<GPUDecodedFrameData>>>>> frames_;
};

} // lightdb::physical

#endif //LIGHTDB_UNIONOPERATORS_H
