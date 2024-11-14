set(HEADERS
    Comp.h
    FFmpegRead.h
    ImageEffect.h
    ImageEffectHost.h
    ImageGraph.h
    ImageNode.h
    MessageLog.h
    Plugin.h
    PropertySet.h
    Read.h
    TimeWarp.h
    Timeline.h
    Util.h)
set(HEADERS_PRIVATE
    ImageEffect_p.h)
set(SOURCE
    Comp.cpp
    FFmpegRead.cpp
    ImageEffect.cpp
    ImageEffectHost.cpp
    ImageGraph.cpp
    ImageNode.cpp
    MessageLog.cpp
    Plugin.cpp
    PropertySet.cpp
    Read.cpp
    TimeWarp.cpp
    Timeline.cpp
    Util.cpp)
if(WIN32)
    list(APPEND SOURCE
        PluginWin32.cpp
        UtilWin32.cpp)
else()
    list(APPEND SOURCE
        PluginUnix.cpp
        UtilUnix.cpp)
endif()

add_library(toucan ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
set(LIBS PUBLIC OTIO::opentimelineio OTIO::opentime OpenImageIO::OpenImageIO)
if(TARGET MINIZIP::minizip)
    list(APPEND LIBS MINIZIP::minizip)
endif()
target_link_libraries(toucan ${LIBS})
set_target_properties(toucan PROPERTIES FOLDER lib)
set_target_properties(toucan PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS toucan
    EXPORT toucanTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/toucan)
install(
    EXPORT toucanTargets
    FILE toucanTargets.cmake
    DESTINATION "lib/cmake/toucan"
    NAMESPACE toucan::)
