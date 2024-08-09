include(ExternalProject)

set(PNG_GIT_REPOSITORY "https://github.com/glennrp/libpng.git")
set(PNG_GIT_TAG "v1.6.39")

set(PNG_SHARED_LIBS ON)
set(PNG_STATIC_LIBS OFF)
if(NOT BUILD_SHARED_LIBS)
    set(PNG_SHARED_LIBS OFF)
    set(PNG_STATIC_LIBS ON)
endif()

set(PNG_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DPNG_SHARED=${PNG_SHARED_LIBS}
    -DPNG_STATIC=${PNG_STATIC_LIBS}
    -DPNG_TESTS=OFF
    -DPNG_ARM_NEON=OFF
    -DSKIP_INSTALL_EXECUTABLES=ON
    -DSKIP_INSTALL_PROGRAMS=ON
    -DSKIP_INSTALL_FILES=ON)

ExternalProject_Add(
    PNG
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/PNG
    DEPENDS ZLIB
    GIT_REPOSITORY ${PNG_GIT_REPOSITORY}
    GIT_TAG ${PNG_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${PNG_ARGS})