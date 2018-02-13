#ifndef LIGHTDB_ERRORS_H
#define LIGHTDB_ERRORS_H

#include <stdexcept>

namespace lightdb::errors {
    class GpuRuntimeError: public std::runtime_error {
    public:
        GpuRuntimeError(const char* message, const char* file, int line, const char* function, const char* info = "")
                : std::runtime_error(message),
                  file_(file), line_(line), func_(function), info_(info)
        { }

        const int   line() const { return line_; }
        const char* file() const { return file_; }
        const char* func() const { return func_; }
        const char* info() const { return info_; }

    private:
        const char* file_;
        const int   line_;
        const char* func_;
        const char* info_;
    };
} // namespace lightdb::errors

#endif //LIGHTDB_ERRORS_H
