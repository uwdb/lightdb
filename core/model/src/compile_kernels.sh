#!/usr/bin/env bash
nvcc -arch=compute_60 -code=sm_60 -cubin kernels.cu
