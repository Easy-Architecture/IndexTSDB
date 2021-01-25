cmake_minimum_required(VERSION 3.17)
project(IndexTSDB)

set(CMAKE_CXX_STANDARD 14)

add_executable(IndexTSDB main.cpp)

file (GLOB files *.cpp */*.cpp)
foreach (file ${files})
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" exe ${file})
    add_executable (${exe} ${file} gloabl.h)
    message (\ \ \ \ --\ src/${exe}.cpp\ will\ be\ compiled\ to\ bin/${exe})
endforeach ()