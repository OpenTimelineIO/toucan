include(ExternalProject)

set(libjpeg-turbo_GIT_REPOSITORY "https://github.com/libjpeg-turbo/libjpeg-turbo.git")
set(libjpeg-turbo_GIT_TAG "3.0.0")

set(libjpeg-turbo_DEPS ZLIB)
if(NOT WIN32)
    list(APPEND libjpeg-turbo_DEPS NASM)
endif()

set(libjpeg-turbo_ENABLE_SHARED ON)
set(libjpeg-turbo_ENABLE_STATIC OFF)
if(NOT BUILD_SHARED_LIBS)
    set(libjpeg-turbo_ENABLE_SHARED OFF)
    set(libjpeg-turbo_ENABLE_STATIC ON)
endif()

set(libjpeg-turbo_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib
    -DENABLE_SHARED=${libjpeg-turbo_ENABLE_SHARED}
    -DENABLE_STATIC=${libjpeg-turbo_ENABLE_STATIC})
if(NOT WIN32)
    list(APPEND libjpeg-turbo_ARGS -DCMAKE_ASM_NASM_COMPILER=${CMAKE_INSTALL_PREFIX}/bin/nasm)
endif()

ExternalProject_Add(
    libjpeg-turbo
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libjpeg-turbo
    DEPENDS ${libjpeg-turbo_DEPS}
    GIT_REPOSITORY ${libjpeg-turbo_GIT_REPOSITORY}
    GIT_TAG ${libjpeg-turbo_GIT_TAG}
    CMAKE_ARGS ${libjpeg-turbo_ARGS})
