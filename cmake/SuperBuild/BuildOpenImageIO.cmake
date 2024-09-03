include(ExternalProject)

set(OpenImageIO_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/OpenImageIO.git")
# Commit : dev: span utility improvements (#4398)
set(OpenImageIO_GIT_TAG "a534a392cd5aa027741f7359d3a7f4799d6b9fcc")

set(OpenImageIO_GIT_REPOSITORY "https://github.com/darbyjohnston/OpenImageIO")
set(OpenImageIO_GIT_TAG "ffmpeg_add_metadata")

set(OpenImageIO_DEPS TIFF PNG libjpeg-turbo OpenEXR OpenColorIO Freetype)
if(toucan_FFMPEG)
    list(APPEND OpenImageIO_DEPS FFmpeg)
endif()

set(OpenImageIO_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DOIIO_BUILD_TESTS=OFF
    -DOIIO_BUILD_TOOLS=OFF
    -DOIIO_BUILD_DOCS=OFF
    -DOIIO_INSTALL_DOCS=OFF
    -DOIIO_INSTALL_FONTS=ON
    -DUSE_FREETYPE=ON
    -DUSE_PNG=ON
    -DUSE_FFMPEG=${toucan_FFMPEG}
    -DUSE_OPENCOLORIO=ON
    -DUSE_BZIP2=OFF
    -DUSE_DCMTK=OFF
    -DUSE_GIF=OFF
    -DUSE_JXL=OFF
    -DUSE_LIBHEIF=OFF
    -DUSE_LIBRAW=OFF
    -DUSE_NUKE=OFF
    -DUSE_OPENCV=OFF
    -DUSE_OPENJPEG=OFF
    -DUSE_OPENVDB=OFF
    -DUSE_PTEX=OFF
    -DUSE_PYTHON=OFF
    -DUSE_QT=OFF
    -DUSE_TBB=OFF
    -DUSE_WEBP=OFF)

ExternalProject_Add(
    OpenImageIO
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenImageIO
    DEPENDS ${OpenImageIO_DEPS}
    GIT_REPOSITORY ${OpenImageIO_GIT_REPOSITORY}
    GIT_TAG ${OpenImageIO_GIT_TAG}
    CMAKE_ARGS ${OpenImageIO_ARGS})
