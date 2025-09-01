include(ExternalProject)

set(OpenTimelineIO_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/OpenTimelineIO.git")
set(OpenTimelineIO_GIT_TAG "d3ca19d802f3c0ea750ab722aa90448a5fdff11f")

set(OpenTimelineIO_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DOTIO_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DOTIO_FIND_IMATH=ON)

ExternalProject_Add(
    OpenTimelineIO
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenTimelineIO
    DEPENDS Imath
    GIT_REPOSITORY ${OpenTimelineIO_GIT_REPOSITORY}
    GIT_TAG ${OpenTimelineIO_GIT_TAG}
    CMAKE_ARGS ${OpenTimelineIO_ARGS})
