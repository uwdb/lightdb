//#include "Tiler.h"
#include "EncodeAPI.h"
//#include "Transcoder.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wregister"
#include <Python.h>
#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
//#include <boost/python/numeric.hpp>

#pragma GCC diagnostic pop

#include <future>
#include <stddef.h>
#include <string>
#include <thread>
#include <unistd.h>

//#include "TileVideoEncoder.h"
#include <ctime>
#include <fstream>
#include <vector>

class Constants {};

namespace Python {
    /*
class Transcoder {
public:
  Transcoder(unsigned int height, unsigned int width, EncodeCodec codec, cudaVideoCodec decodeCodec, std::string preset, unsigned int fps,
             unsigned int gop_length, unsigned long bitrate, NV_ENC_PARAMS_RC_MODE rcmode, unsigned int deviceId)
      : context(deviceId),
        encodeConfiguration(Configuration{width, height, 0, 0, bitrate, {fps, 1}}, codec, preset.data(), gop_length, rcmode),
        decodeConfiguration(encodeConfiguration, lightdb::Codec::hevc()), //decodeCodec),
        gpuTranscoder(context, decodeConfiguration, encodeConfiguration) {
      //: gpuTranscoder(height, width, codec, preset, fps, gop_length, bitrate, rcmode, deviceId) {
    //if (gpuTranscoder.initialize() != 0)
      //throw std::runtime_error("Transcoder initialization error");
  }

  PyObject *transcode(PyObject *inputBuffer) {
    Py_buffer *resultBuffer;
    char inputFilename[] = "/tmp/transcodeInputXXXXXX";
    char outputFilename[] = "/tmp/transcodeOutputXXXXXX";
    char *inputData, *outputData;
    int inputDescriptor, outputDescriptor;
    ssize_t inputDataLength, outputDataLength;

    if (PyBytes_AsStringAndSize(inputBuffer, &inputData, &inputDataLength) != 0)
      throw std::runtime_error("PyBytes_AsStringAndSize");
    else if ((inputDescriptor = mkstemp(inputFilename)) < 0)
      throw std::runtime_error("Input mkstemp");
    else if (write(inputDescriptor, inputData, inputDataLength) != inputDataLength)
      throw std::runtime_error("Input write");
    else if (close(inputDescriptor) != 0)
      throw std::runtime_error("Input close");
    else if ((outputDescriptor = mkstemp(outputFilename)) < 0)
      throw std::runtime_error("output mkstemp");

    printf("output filename: %s\n", outputFilename);

    FileDecodeReader reader(inputFilename);
    FileEncodeWriter writer(gpuTranscoder.encoder().api(), outputFilename);

    gpuTranscoder.transcode(reader, writer);
    //if (gpuTranscoder.transcode(std::string(inputFilename), std::string(outputFilename)) != 0)
      //throw std::runtime_error("transcode error");
    if ((outputDataLength = lseek(outputDescriptor, 0, SEEK_END)) < 0)
      throw std::runtime_error("Output length seek to end");
    else if (lseek(outputDescriptor, 0, SEEK_SET) != 0)
      throw std::runtime_error("Output length seek to start");
    else if ((outputData = new char[outputDataLength]) == nullptr)
      throw std::runtime_error("Output buffer allocation");
    else if (read(outputDescriptor, outputData, outputDataLength) != outputDataLength)
      throw std::runtime_error("Output read underflow");
    else if (close(outputDescriptor) != 0)
      throw std::runtime_error("Output close");
    else if (remove(inputFilename) != 0)
      throw std::runtime_error("Input remove");
    else if (remove(outputFilename) != 0)
      throw std::runtime_error("Output remove");
    else if ((resultBuffer = new Py_buffer()) == nullptr)
      throw std::runtime_error("allocate buffer fail");
    else if (PyBuffer_FillInfo(resultBuffer, nullptr, outputData, outputDataLength, true, PyBUF_CONTIG_RO) != 0)
      throw std::runtime_error("PyBuffer_FillInfo");
    else
      return PyMemoryView_FromBuffer(resultBuffer);
  }

private:
    GPUContext context;
    EncodeConfiguration encodeConfiguration;
    DecodeConfiguration decodeConfiguration;
  ::Transcoder gpuTranscoder;
};

class Tiler {
public:
  Tiler(unsigned int height, unsigned int width, unsigned int tileRows, unsigned int tileColumns, PyObject *qualities,
        unsigned int codec, unsigned int fps, unsigned int gop_length, unsigned int deviceId)
      : configurations(getConfigurations(inputFilename, height, width, tileRows,
                                         tileColumns, // TODO change parameter to TileDimensions&
                                         qualities, codec, fps, gop_length, deviceId)),
        tileRows(tileRows), tileColumns(tileColumns)
        //tileDimensions{tileRows, tileColumns, tileRows * tileColumns}
  { }

  ~Tiler() {
    //for (auto &configuration : configurations)
    //  delete configuration.outputFileName;
  }

  static std::vector<EncodeConfiguration> getConfigurations(char *inputFilename, unsigned int height, unsigned int width,
                                                     unsigned int tileRows, unsigned int tileColumns,
                                                     PyObject *qualities, unsigned int codec, unsigned int fps,
                                                     unsigned int gop_length, unsigned int deviceId) {
    auto *sequence = PySequence_Fast(qualities, "expected a sequence");
    auto length = PySequence_Size(sequence);
    std::vector<EncodeConfiguration> configurations;

    for (auto i = 0; i < length; i++) {
      //auto quality = PySequence_Fast_GET_ITEM(sequence, i);
      //auto bitrate = PyLong_AsLong(PyObject_GetAttrString(quality, "bitrate")) * 1024;
      //auto rate_control_mode = PyLong_AsLong(PyObject_GetAttrString(quality, "rate_control_mode"));
      //auto *preset = PyString_AsString(PyObject_GetAttrString(quality, "preset"));
      std::string outputFilenameFormat;

      outputFilenameFormat.reserve(65);
      snprintf(outputFilenameFormat.data(), 64, outputFilenameFormatTemplate, i);

//      configurations.push_back(MakeTilerConfiguration(inputFilename, outputFilenameFormat, height, width, tileRows,
  //                                                    tileColumns, codec, preset, fps, gop_length, bitrate,
    //                                                  rate_control_mode, deviceId));
    }

    Py_DECREF(sequence);

    return configurations;
  }

  void start(PyObject *inputBuffer) {
    char *inputData;
    ssize_t inputDataLength;

    strcpy(inputFilename, inputFilenameTemplate);

    if (PyBytes_AsStringAndSize(inputBuffer, &inputData, &inputDataLength) != 0)
      throw std::runtime_error("PyBytes_AsStringAndSize");
    else {
      tileFuture = std::async(std::launch::async, [this, inputData, inputDataLength] {
                     int inputDescriptor;

                     if ((inputDescriptor = mkstemp(inputFilename)) < 0)
                       throw std::runtime_error("Input mkstemp");
                     else if (write(inputDescriptor, inputData, inputDataLength) != inputDataLength)
                       throw std::runtime_error("Input write");
                     else if (close(inputDescriptor) != 0)
                       throw std::runtime_error("Input close");
                     //else if (ExecuteTiler(configurations, tileDimensions) != 0)
                     //  throw std::runtime_error("tiler");
                     else
                       return true;
                   }).share();
      ;
    }
  }

  bool complete() { return tileFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready; }

  //PyObject *tiles() { return make_list(tileDimensions.count, configurations.size()); }

  static PyObject *make_list(const size_t tile_count, const size_t quality_count) {
    Py_buffer *buffer;
    PyObject *list = PyList_New(tile_count * quality_count); // TODO leaks on failure
    char filename[1024], *outputData;
    long outputFileSize;
    FILE *outputFile;

    for (auto quality = 0u; quality < quality_count; quality++)
      for (auto tile = 0u; tile < tile_count; tile++) {
        if (snprintf(filename, sizeof(filename), outputFilenameFormat, quality, tile) < 0)
          throw std::runtime_error("snprintf");
        else if ((outputFile = fopen(filename, "rb")) == nullptr)
          throw std::runtime_error("fopen");
        else if (fseek(outputFile, 0, SEEK_END) != 0)
          throw std::runtime_error("fseek");
        else if ((outputFileSize = ftell(outputFile)) < 0)
          throw std::runtime_error("ftell");
        else if (fseek(outputFile, 0, SEEK_SET) != 0)
          throw std::runtime_error("Output length seek to start");
        else if ((outputData = new char[outputFileSize]) == nullptr)
          throw std::runtime_error("Output buffer allocation");
        else if (fread(outputData, 1, static_cast<size_t>(outputFileSize), outputFile) != static_cast<size_t>(outputFileSize))
          throw std::runtime_error("Output read underflow");
        else if (fclose(outputFile) != 0)
          throw std::runtime_error("Output close");
        else if (remove(filename) != 0)
          throw std::runtime_error("Output remove");
        else if ((buffer = new Py_buffer()) == nullptr)
          throw std::runtime_error("new pybuffer"); // leaks outputData
        else if (PyBuffer_FillInfo(buffer, nullptr, outputData, outputFileSize, true, PyBUF_CONTIG_RO) != 0)
          throw std::runtime_error("PyBuffer_FillInfo");
        else
          PyList_SET_ITEM(list, tile + (tile_count * quality), PyMemoryView_FromBuffer(buffer));
      }

    return list;
  }

private:
  static constexpr const char *outputFilenameFormatTemplate = "/tmp/tilerOutput-quality%d-tile%%d";
  static constexpr const char *outputFilenameFormat = "/tmp/tilerOutput-quality%d-tile%d";
  static constexpr const char *inputFilenameTemplate = "/tmp/tilerInputXXXXXX";
  char inputFilename[64]; // TODO these should all be static, fix this
  std::shared_future<bool> tileFuture;
  std::vector<EncodeConfiguration> configurations;
    const size_t tileRows, tileColumns;
  //TileDimensions tileDimensions;
};

BOOST_PYTHON_MODULE(dashtranscoder) {
  boost::python::class_<Tiler>("Tiler",
                               boost::python::init<unsigned int, unsigned int, unsigned int, unsigned int, PyObject *,
                                                   unsigned int, unsigned int, unsigned int, unsigned int>())
      .def("start", &Tiler::start)
      .def("complete", &Tiler::complete);
      //.def("tiles", &Tiler::tiles);
  //.def("tile2", &Tiler::tile2);
  //.def("tile_all", &Tiler::tile_all);

//  boost::python::class_<Transcoder>(
  //    "Transcoder", boost::python::init<unsigned int, unsigned int, unsigned int, std::string, unsigned int,
    //                                    unsigned int, unsigned long, unsigned int, unsigned int>())
      //.def("transcode", &Transcoder::transcode);

  boost::python::scope constants = boost::python::class_<Constants>("constants");

  constants.attr("H264") = static_cast<unsigned int>(NV_ENC_H264);
  constants.attr("H265") = static_cast<unsigned int>(NV_ENC_HEVC);
}
*/
     }