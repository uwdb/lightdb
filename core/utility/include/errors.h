#ifndef LIGHTDB_ERRORS_H
#define LIGHTDB_ERRORS_H

#include "EncodeAPI.h"
#include <glog/logging.h>
#include <stdexcept>
#include <utility>

#define InvalidArgumentError(message, argument) \
    ::lightdb::errors::_InvalidArgument(message, argument, __FILE__, __LINE__, __func__)
#define BadCastError(message) \
    ::lightdb::errors::_BadCast(__FILE__, __LINE__, __func__, "" message)

#define NotImplementedError(message) \
    ::lightdb::errors::_NotImplemented(__FILE__, __LINE__, __func__, "Not implemented " message)

#define CatalogError(message, name) \
    ::lightdb::errors::_CatalogError(message, name, __FILE__, __LINE__, __func__)
#define SerializationError(message) \
    ::lightdb::errors::_SerializationError(message, __FILE__, __LINE__, __func__)

#define CoordinatorError(message) \
    ::lightdb::errors::_CoordinatorError(message, __FILE__, __LINE__, __func__)

#define FfmpegRuntimeError(message)                \
    ::lightdb::errors::_FfmpegRuntimeError(message, __FILE__, __LINE__, __func__)
#define GpacRuntimeError(message, status)                \
    ::lightdb::errors::_GpacRuntimeError(message, status, __FILE__, __LINE__, __func__)

#define GpuRuntimeError(message)                \
    ::lightdb::errors::_GpuRuntimeError(message, __FILE__, __LINE__, __func__)
#define GpuEncodeRuntimeError(message, status)  \
    ::lightdb::errors::_GpuEncodeRuntimeError(message, status, __FILE__, __LINE__, __func__)
#define GpuCudaRuntimeError(message, status)    \
    ::lightdb::errors::_GpuCudaRuntimeError(message, status, __FILE__, __LINE__, __func__)

#define PluginError(message, name) \
    ::lightdb::errors::_PluginError(message, name, __FILE__, __LINE__, __func__)

namespace lightdb::errors {
    template<typename Exception>
    class LightDBError: public Exception {
    protected:
        LightDBError(const std::string &message, const char* file, int line, const char* function)
            : Exception(message), file_(file), line_(line), func_(function)
        { log(); }

        int         line() const { return line_; }
        const char* file() const { return file_; }
        const char* function() const { return func_; }

    protected:
        virtual void log() const { LOG(ERROR) << this->what() << " (" << function() << ':' << file() << ':' << line() << ')'; }

    private:
        const char* file_;
        const int   line_;
        const char* func_;
    };

    class _BadCast: public std::bad_cast {
    public:
        _BadCast(const char* file, int line, const char* function, const char *message = nullptr)
                : _BadCast(file, line, function, message == nullptr ? "" : std::string{message})
        { }

        _BadCast(const char* file, int line, const char* function, std::string message = "")
                : file_(file), line_(line), func_(function), message_(std::move(message))
        { log(); }

        int         line() const { return line_; }
        const char* file() const { return file_; }
        const char* function() const { return func_; }
        const std::string message() const { return !message_.empty() ? message_ : what(); }

    protected:
        virtual void log() const { LOG(ERROR) << message() << " (" << function() << ':' << file() << ':' << line() << ')'; }

    private:
        const char* file_;
        const int   line_;
        const char* func_;
        const std::string message_;
    };

    class _InvalidArgument: public LightDBError<std::invalid_argument> {
    public:
        _InvalidArgument(const std::string &message, std::string argument, const char* file, int line, const char* function)
            : LightDBError(message, file, line, function), argument_(std::move(argument))
        {  }

        const std::string& argument() const { return argument_; }

    protected:
        void log() const override {
            LOG(ERROR) << argument() << ':' << what() << " (" << function() << ':' << file() << ':' << line() << ')';
        }

    private:
        const std::string argument_;
    };

    class _NotImplemented: public LightDBError<std::runtime_error> {
    public:
        _NotImplemented(const char* file, int line, const char* function, const char *message = nullptr)
                : LightDBError(message ? message : "Not implemented", file, line, function)
        {  }
    };

    class _CatalogError: public LightDBError<std::runtime_error> {
    public:
        _CatalogError(const std::string &message, std::string name, const char* file, int line, const char* function)
                : LightDBError(message, file, line, function), name_(std::move(name))
        {  }

        const std::string& name() const { return name_; }

    protected:
        void log() const override {
            LOG(ERROR) <<  name() << ':' << what() << " (" << function() << ':' << file() << ':' << line() << ')';
        }

    private:
        const std::string name_;
    };

    class _SerializationError: public LightDBError<std::runtime_error> {
    public:
        _SerializationError(const char *message, const char* file, int line, const char* function)
                : LightDBError(message ? message : "Serialization error", file, line, function)
        {  }
    };

    class _CoordinatorError: public LightDBError<std::runtime_error> {
    public:
        _CoordinatorError(const std::string &message, const char* file, int line, const char* function)
                : LightDBError(message, file, line, function)
        {  }

    protected:
        void log() const override {
            LOG(ERROR) << "Coordinator:" << what() << " (" << function() << ':' << file() << ':' << line() << ')';
        }
    };

    class _FfmpegRuntimeError: public LightDBError<std::runtime_error> {
    public:
        _FfmpegRuntimeError(const std::string &message, const char* file, int line, const char* function)
                : LightDBError(message, file, line, function)
        { }

        _FfmpegRuntimeError(const int &error, const char* file, int line, const char* function)
                : _FfmpegRuntimeError(get_ffmpeg_error_message(error), file, line, function)
        { }

    private:
        std::string get_ffmpeg_error_message(int);
    };

    class _GpacRuntimeError: public LightDBError<std::runtime_error> {
    public:
        _GpacRuntimeError(const std::string &message, unsigned int status, const char* file, int line, const char* function)
                : LightDBError(message, file, line, function),
                  status_(status)
        { }

        unsigned int status() const { return status_; }

    private:
        const unsigned int status_;
    };

    class _GpuRuntimeError: public LightDBError<std::runtime_error> {
    public:
        _GpuRuntimeError(const std::string &message, const char* file, int line, const char* function)
            : LightDBError(message, file, line, function)
        { }
    };

    class _GpuEncodeRuntimeError: public _GpuRuntimeError {
    public:
        _GpuEncodeRuntimeError(const std::string &message, NVENCSTATUS status,
                              const char* file, int line, const char* function)
            : _GpuRuntimeError(message + " (NVENCSTATUS=" + std::to_string(status) + ')', file, line, function),
              status_(status)
        { }

        NVENCSTATUS status() const { return status_; }

    private:
        NVENCSTATUS status_;
    };

    class _GpuCudaRuntimeError: public _GpuRuntimeError {
    public:
        _GpuCudaRuntimeError(const std::string &message, CUresult status,
                               const char* file, int line, const char* function)
            : _GpuRuntimeError(message + " (CUresult=" + std::to_string(status) + ')', file, line, function),
              status_(status)
        { }

        CUresult status() const { return status_; }

    private:
        CUresult status_;
    };

    class _PluginError: public LightDBError<std::runtime_error> {
    public:
        _PluginError(const std::string &message, std::string name, const char* file, int line, const char* function)
                : LightDBError(message, file, line, function), name_(std::move(name))
        {  }

        const std::string& name() const { return name_; }

    protected:
        void log() const override {
            LOG(ERROR) <<  name() << ':' << what() << " (" << function() << ':' << file() << ':' << line() << ')';
        }

    private:
        const std::string name_;
    };
} // namespace lightdb::errors

#endif //LIGHTDB_ERRORS_H
