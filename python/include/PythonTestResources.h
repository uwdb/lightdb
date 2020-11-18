#ifndef LIGHTDB_PYTHON_TESTRESOURCES_H
#define LIGHTDB_PYTHON_TESTRESOURCES_H

#include TestResources.h
#include <boost/python.hpp>


BOOST_PYTHON_MODULE (pylightdb_test) {
    boost::python::class_<Resources>("Resources")
        .def_readonly("catalog_name", &Resources::catalog_name);
    boost::python::class_<Resources::red10>("red10")
        .def_readonly("name", &Resources::red10::name)
        .def_readonly("metadata_path", &Resources::red10::metadata_path)
        .def_readonly("height", &Resources::red10::height)
        .def_readonly("width", &Resources::red10::width)
        .def_readonly("frames", &Resources::red10::frames)
        .def_readonly("framerate", &Resources::red10::framerate);
    boost::python::class_<Resources::green10>("green10")
        .def_readonly("name", &Resources::green10::name)
        .def_readonly("height", &Resources::green10::height)
        .def_readonly("width", &Resources::green10::width)
        .def_readonly("frames", &Resources::green10::frames)
        .def_readonly("framerate", &Resources::green10::framerate);
    boost::python::class_<Resources::videos::black>("videosBlack")
        .def_readonly("height", &Resources::videos::black::height)
        .def_readonly("width", &Resources::videos::black::width)
        .def_readonly("frames", &Resources::videos::black::frames)
        .def_readonly("duration", &Resources::videos::black::duration);
    boost::python::class_<Resources::videos::black::h264>("videosBlackH264")
        .def_readonly("name", &Resources::videos::black::h264::name);
    boost::python::class_<Resources::videos::black::mp4>("videosBlackMP4")
        .def_readonly("name", &Resources::videos::black::mp4::name)
        .def_readonly("codec", &Resources::videos::black::mp4::codec)
        .def_readonly("fps", &Resources::videos::black::mp4::fps);
    boost::python::class_<Resources::out>("out")
        .def_readonly("hevc", &Resources::out::hevc)
        .def_readonly("h264", &Resources::out::h264)
        .def_readonly("raw", &Resources::out::raw);
    boost::python::class_<Resources::datasets::timelapse>("timelapse")
        .def_readonly("path", &Resources::datasets::timelapse::path)
        .def_readonly("timelapse1k", &Resources::datasets::timelapse::timelapse1k)
        .def_readonly("timelapse2k", &Resources::datasets::timelapse::timelapse2k)
        .def_readonly("timelapse4k", &Resources::datasets::timelapse::timelapse4k);
    boost::python::class_<Resources::datasets::ua_detrac>("ua_detrac")
        .def_readonly("path", &Resources::datasets::ua_detrac::path);
    boost::python::class_<Resources::datasets::visualroad>("visualroad")
        .def_readonly("path", &Resources::datasets::visualroad::path);
    boost::python::class_<Resources::datasets::random>("random_datasets")
        .def_readonly("path", &Resources::datasets::random::path);
    boost::python::class_<Resources::plugins::blur>("plugins")
        .def_readonly("name", &Resources::plugins::blur::name);
        // ToDo: add greyscale and update blur to python blur
}       


#endif //LIGHTDB_PYTHON_TESTRESOURCES_H