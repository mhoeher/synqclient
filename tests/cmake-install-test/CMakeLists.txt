cmake_minimum_required(VERSION 3.0)
project(synqclient-cmake-install-test)

find_package(SynqClient-qt5 REQUIRED)

add_executable(synqclient-cmake-install-test)
target_sources(synqclient-cmake-install-test PRIVATE main.cpp)
target_link_libraries(synqclient-cmake-install-test PRIVATE SynqClient::synqclient-qt5)
