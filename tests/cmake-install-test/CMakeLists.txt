cmake_minimum_required(VERSION 3.0)
project(synqclient-cmake-install-test)

find_package(SynqClient REQUIRED)

add_executable(synqclient-cmake-install-test)
target_sources(synqclient-cmake-install-test PRIVATE main.cpp)
target_link_libraries(synqclient-cmake-install-test PRIVATE SynqClient::synqclient)
