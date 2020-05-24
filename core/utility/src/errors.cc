#include "errors.h"

extern "C" {
#include <libavutil/error.h>
};

namespace lightdb::errors {
    std::string _FfmpegRuntimeError::get_ffmpeg_error_message(int error) {
        char message[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(error, message, sizeof(message));
        return message;
    }

}