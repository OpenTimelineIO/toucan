include(ExternalProject)

set(feather_tk_GIT_REPOSITORY "https://github.com/darbyjohnston/feather-tk.git")
set(feather_tk_GIT_TAG "f8103df2aa07a3722930c19ee9093817dd1af0c8")

set(feather_tk_DEPS ZLIB nlohmann_json PNG Freetype lunasvg glfw3)
if(toucan_nfd)
    list(APPEND feather_tk_DEPS nfd)
endif()

set(feather_tk_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -Dfeather_tk_UI_LIB=ON
    -Dfeather_tk_PYTHON=OFF
    -Dfeather_tk_TESTS=OFF
    -Dfeather_tk_EXAMPLES=OFF)

ExternalProject_Add(
    feather_tk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/feather_tk
    DEPENDS ${feather_tk_DEPS}
    GIT_REPOSITORY ${feather_tk_GIT_REPOSITORY}
    GIT_TAG ${feather_tk_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${feather_tk_ARGS})
