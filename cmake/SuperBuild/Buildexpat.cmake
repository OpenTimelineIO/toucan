include(ExternalProject)

set(expat_GIT_REPOSITORY "https://github.com/libexpat/libexpat.git")
set(expat_GIT_TAG "R_2_5_0")

set(expat_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DEXPAT_BUILD_TOOLS=OFF
    -DEXPAT_BUILD_EXAMPLES=OFF
    -DEXPAT_BUILD_TESTS=OFF)

ExternalProject_Add(
    expat
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/expat
    GIT_REPOSITORY ${expat_GIT_REPOSITORY}
    GIT_TAG ${expat_GIT_TAG}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/expat-patch/expat/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/expat/src/expat/expat/CMakeLists.txt
    SOURCE_SUBDIR expat
    LIST_SEPARATOR |
    CMAKE_ARGS ${expat_ARGS})
