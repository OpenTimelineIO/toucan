include(ExternalProject)

set(ftk_GIT_REPOSITORY "https://github.com/darbyjohnston/feather-tk.git")
set(ftk_GIT_TAG "d3d0eed9f1a77782f56d4ce3f74eb2260c0b49ca")

set(ftk_DEPS ZLIB nlohmann_json PNG Freetype lunasvg)

set(ftk_sbuild_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -Dftk_ZLIB=OFF
    -Dftk_nlohmann_json=OFF
    -Dftk_PNG=OFF
    -Dftk_Freetype=OFF
    -Dftk_lunasvg=OFF)

ExternalProject_Add(
    ftk-sbuild
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk-sbuild
    DEPENDS ${ftk_DEPS}
    GIT_REPOSITORY ${ftk_GIT_REPOSITORY}
    GIT_TAG ${ftk_GIT_TAG}
    INSTALL_COMMAND ""
    SOURCE_SUBDIR etc/SuperBuild
    LIST_SEPARATOR |
    CMAKE_ARGS ${ftk_sbuild_ARGS})

set(ftk_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -Dftk_TESTS=OFF
    -Dftk_EXAMPLES=OFF)

ExternalProject_Add(
    ftk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk
    DEPENDS ftk-sbuild
    GIT_REPOSITORY ${ftk_GIT_REPOSITORY}
    GIT_TAG ${ftk_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${ftk_ARGS})
