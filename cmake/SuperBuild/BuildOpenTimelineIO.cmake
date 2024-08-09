include(ExternalProject)

set(OpenTimelineIO_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/OpenTimelineIO.git")
set(OpenTimelineIO_GIT_TAG "v0.17.0")

set(OpenTimelineIO_ARGS
    -DOTIO_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DOTIO_FIND_IMATH=ON
    ${toucan_EXTERNAL_PROJECT_ARGS})

ExternalProject_Add(
    OpenTimelineIO
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenTimelineIO
    DEPENDS Imath
    GIT_REPOSITORY ${OpenTimelineIO_GIT_REPOSITORY}
    GIT_TAG ${OpenTimelineIO_GIT_TAG}
    CMAKE_ARGS ${OpenTimelineIO_ARGS})
