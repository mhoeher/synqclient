cmake_minimum_required(VERSION 3.17)
set(SYNQCLIENT_VERSION 0.0.0)
project(SynqClient VERSION ${SYNQCLIENT_VERSION})


# TODO: Version from git

include(GNUInstallDirs)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SYNQCLIENT_WITHOUT_TESTS "Build without unit tests" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(
    SYNQCLIENT_REQUIRED_QT_DEPENDENCIES
    Core Network Xml Sql
)
find_package(QT NAMES Qt6 Qt5 COMPONENTS ${SYNQCLIENT_REQUIRED_QT_DEPENDENCIES} Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SYNQCLIENT_REQUIRED_QT_DEPENDENCIES} Test REQUIRED)


add_subdirectory(libsynqclient)

if(NOT SYNQCLIENT_WITHOUT_TESTS)
    add_subdirectory(tests)
endif()


# Export targets:
install(
    EXPORT
        SynqClientTargets
    NAMESPACE
        SynqClient::
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/SynqClient
)

# Export library interface:
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    SynqClientConfigVersion.cmake
    VERSION
        ${SYNQCLIENT_VERSION}
    COMPATIBILITY
        SameMajorVersion
)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/SynqClientConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/SynqClientConfigVersion.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/SynqClient
)
configure_package_config_file(
    cmake/SynqClientConfig.cmake SynqClientConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SynqClient
)


# Also create a *.pri file and install it, so qmake based projects
# can more easily build against the library:
find_package(ECM)
if(ECM_FOUND)
    list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_DIR}")
    include(ECMGeneratePriFile)
    set(PROJECT_VERSION_STRING ${SYNQCLIENT_VERSION})
    unset(PRI_DEPS)
    foreach(dep ${SYNQCLIENT_REQUIRED_QT_DEPENDENCIES})
        string(TOLOWER ${dep} dep)
        if(PRI_DEPS)
            set(PRI_DEPS "${PRI_DEPS} ${dep}")
        else()
            set(PRI_DEPS ${dep})
        endif()
    endforeach()
    
    ecm_generate_pri_file(
        BASE_NAME
            SynqClient
        LIB_NAME
            synqclient
        DEPS
            "${PRI_DEPS}"
        FILENAME_VAR
            SYNQCLIENT_PRI_FILE
    )
    install(
        FILES 
            ${SYNQCLIENT_PRI_FILE} 
        DESTINATION 
            ${ECM_MKSPECS_INSTALL_DIR}
    )
endif()