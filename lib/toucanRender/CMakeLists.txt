set(HEADERS
    Comp.h
    FFmpeg.h
    FFmpegRead.h
    FFmpegWrite.h
    ImageEffect.h
    ImageEffectHost.h
    ImageGraph.h
    ImageNode.h
    MemoryMap.h
    Plugin.h
    PropertySet.h
    Read.h
    TimeWarp.h
    TimelineAlgo.h
    TimelineWrapper.h
    Util.h)
set(HEADERS_PRIVATE
    ImageEffect_p.h)
set(SOURCE
    Comp.cpp
    FFmpeg.cpp
    FFmpegRead.cpp
    FFmpegWrite.cpp
    ImageEffect.cpp
    ImageEffectHost.cpp
    ImageGraph.cpp
    ImageNode.cpp
    MemoryMap.cpp
    Plugin.cpp
    PropertySet.cpp
    Read.cpp
    TimeWarp.cpp
    TimelineAlgo.cpp
    TimelineWrapper.cpp
    Util.cpp)
if(WIN32)
    list(APPEND SOURCE
        MemoryMapWin32.cpp
        PluginWin32.cpp)
else()
    list(APPEND SOURCE
        MemoryMapUnix.cpp
        PluginUnix.cpp)
endif()

add_library(toucanRender ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
set(LIBS_PUBLIC
    feather-tk::feather-tk-core
    OTIO::opentimelineio
    OTIO::opentime
    OpenImageIO::OpenImageIO
    MINIZIP::minizip)
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    list(APPEND LIBS_PUBLIC stdc++fs)
endif()
target_link_libraries(toucanRender PUBLIC ${LIBS_PUBLIC})
set_target_properties(toucanRender PROPERTIES FOLDER lib)
set_target_properties(toucanRender PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS toucanRender
    EXPORT toucanRenderTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/toucanRender)
install(
    EXPORT toucanRenderTargets
    FILE toucanRenderTargets.cmake
    DESTINATION "lib/cmake/toucan"
    NAMESPACE toucan::)
