cmake_minimum_required(VERSION 3.17)
project(IndexTSDB)

set(CMAKE_CXX_STANDARD 14)


#add_executable(epollServer main.cpp epollServer/epollServer.cpp epollServer/epollServer.h gloabl.h )
add_executable(epollServer main.cpp epollServer/epollReactorServer.cpp ../../../CProject/senior/socket/epollServer.h gloabl.h )
