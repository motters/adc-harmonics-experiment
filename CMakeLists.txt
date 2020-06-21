cmake_minimum_required(VERSION 3.16)
project(Harmonics)

set(CMAKE_CXX_STANDARD 17)

INCLUDE_DIRECTORIES(
        kissfft
        kissfft/tools
)

add_executable(Harmonics main.cpp kissfft/kiss_fft.c kissfft/kiss_fft.h kissfft/tools/kiss_fftr.h kissfft/tools/kiss_fftr.c)