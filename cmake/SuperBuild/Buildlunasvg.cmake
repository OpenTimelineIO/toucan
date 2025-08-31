include(ExternalProject)
include(ExternalProject)

set(lunasvg_GIT_REPOSITORY "https://github.com/sammycage/lunasvg.git")
set(lunasvg_GIT_TAG "v3.4.0")

set(lunasvg_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DLUNASVG_BUILD_EXAMPLES=OFF)

ExternalProject_Add(
    lunasvg
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/lunasvg
    GIT_REPOSITORY ${lunasvg_GIT_REPOSITORY}
    GIT_TAG ${lunasvg_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${lunasvg_ARGS})
