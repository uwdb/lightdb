#include "PhysicalOperators.h"
#include "OldPhysicalOperators.h"
#include "Operators.h"
#include <LFVideo.h>
#include <glog/logging.h>

namespace lightdb {
    namespace physical {
        //static CLFVideo lfv("/home/bhaynes/projects/light-field-video/data/cats/");

        //template<typename ColorSpace>
        EncodedLightField PlanarTiledToVideoLightField::apply(const std::string &format) {
            CLFVideo lfv("/home/bhaynes/projects/light-field-video/data/cats/");

            auto frames = 100u;
            auto fps = 30u;
            auto width = 512, height = 352;
            double duration = frames/fps;
            VideoWriter writer("planar.mp4", 0x21, fps, Size{width, height}, true);

            for(auto i = 0u; i < frames; i++) {
                auto frame = lfv.RenderLF(0, 0.5, 0.5);

                    //auto frame = lfv.SingleView(i, x_, y_);
                writer.write(frame);
            }

            writer.release();
            system("ffmpeg -hide_banner -loglevel error -y -i planar.mp4 -c copy planar.h264");
            remove("planar.mp4");

            return SingletonFileEncodedLightField::create("planar.h264", {{x_, x_,}, {y_, y_}, {0, 0}, {0, duration}, theta_, phi_});
        }

        //template class PlanarTiledToVideoLightField<YUVColorSpace>;
    } // namespace physical
} // namespace lightdb

