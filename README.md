A SynqClient is a C++ library based on Qt which implements job-based access to file sharing services. On top, it implements synchronization between a local folder structure and a remote one on a server.


# Background

SynqClient is the result of factoring the WebDAV sync code out of the [OpenTodoList](https://opentodolist.rpdev.net/) project. The result is a library which:

- Implements a job based approach to access a storage server.
- On top of this job system, a simple file sync protocol is implemented.

Hence, you can use the library both for simple remote file accesses as well as if you want your app to be able to sync its data across devices.


# Documentation

You can find the API documentation on https://synqclient.readthedocs.io/en/latest/index.html.


# Building

Building the library can be done either with `cmake` or `qmake`.

You will need the following dependencies being installed:

- Qt 5.15 or 6. The following modules are used:
  - Core
  - Network
  - XML
  - SQL

In addition, depending on how you build, the following extra modules can be used:

- Extra CMake Modules (ECM)


## Building with `cmake`

Building using `cmake` is as easy as:

```bash
cd path/to/synqclient
mkdir build
cd build
cmake ..
cmake --build .
cmake --install .
```

*Note:* You must use `cmake` version 3.

To build and link against the library, simple use the following in your `cmake` code:

```cmake
find_package(SynqClient REQUIRED)

target_link_libraries(my-app PUBLIC SynqClient::synqclient)
```

In addition, if `ECM` is found during the build, a `qmake` module file will be created as well. Hence, if you are using a `qmake` based project, you can simple use

```qmake
QT += SynqClient
```

to include the library in your code.

*Note:* If you want to install into the system location, you most probably want to set `ECM_MKSPECS_INSTALL_DIR` appropriately for your system. Please refer to the [ECMGeneratePriFile](https://api.kde.org/ecm/module/ECMGeneratePriFile.html) documentation for more details.


### Fine Tuning The Build

The following options can be passed to `cmake` (e.g. via `-DOPTION_NAME=ON`) to tweak the build:

| Flag | Description |
| ---- | ----------- |
| SYNQCLIENT_WITHOUT_TESTS | Do not build the unit tests. |


## Building with `qmake`

The `qmake` based build is also pretty easy:

```bash
cd path/to/synqclient
mkdir build
cd build
qmake ..
make
make install
```

*Important:* The `qmake` build will neither yield `qmake` not `cmake` integration files. It is recommended only if you have a `qmake` based project and want to build SynqClient as part of your project build.

### Fine Tuning The Build

The following configuration switched can be passed to `qmake` (e.g. pass `CONFIG+=XXX` when calling it):

| Flag | Description |
| ----- | ------------ |
| synqclient_with_no_tests | Do not build the unit tests. |
| synqclient_with_static_libs | Build the library as a static library. |
