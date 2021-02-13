cmake_minimum_required(VERSION 3.17)
project(SynqClient)

set(SYNQCLIENT_VERSION 0.0.0)

# TODO: Version from git

include(GNUInstallDirs)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SYNQCLIENT_WITHOUT_TESTS "Build without unit tests" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Xml Sql Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network Xml Sql Test REQUIRED)

add_subdirectory(libsynqclient)

if(NOT SYNQCLIENT_WITHOUT_TESTS)
    add_subdirectory(tests)
endif()
