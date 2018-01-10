#!/usr/bin/env bash
#sudo update-alternatives --set gcc /usr/bin/gcc-4.9
nvcc -ccbin /usr/bin/gcc-4.9 -arch=compute_60 -code=sm_60 -cubin /home/bhaynes/projects/visualcloud/core/model/src/kernels.cu -odir /home/bhaynes/projects/visualcloud/core/model/src
#sudo update-alternatives --auto gcc
