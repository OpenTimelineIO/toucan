include(ExternalProject)

set(pystring_GIT_REPOSITORY "https://github.com/imageworks/pystring.git")
set(pystring_GIT_TAG "02ef1186d6b77bc35f385bd4db2da75b4736adb7")

set(pystring_ARGS ${toucan_EXTERNAL_PROJECT_ARGS})

ExternalProject_Add(
    pystring
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/pystring
    GIT_REPOSITORY ${pystring_GIT_REPOSITORY}
    GIT_TAG ${pystring_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${pystring_ARGS})
