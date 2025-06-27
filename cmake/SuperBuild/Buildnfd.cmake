include(ExternalProject)

set(nfd_GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git")
set(nfd_GIT_TAG "v1.2.1")

set(nfd_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DNFD_USE_ALLOWEDCONTENTTYPES_IF_AVAILABLE=OFF
    -DNFD_BUILD_TESTS=OFF)

ExternalProject_Add(
    nfd
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/nfd
    GIT_REPOSITORY ${nfd_GIT_REPOSITORY}
    GIT_TAG ${nfd_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${nfd_ARGS})
