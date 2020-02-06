FROM lightdb/environment
MAINTAINER Brandon Haynes "bhaynes@cs.washington.edu"

ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES compute,utility,video

COPY . /lightdb

RUN mkdir build && cd build && cmake .. && make
