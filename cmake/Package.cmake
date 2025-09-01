 set(CPACK_PACKAGE_FILE_NAME
    ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})

if(WIN32)
    set(CPACK_GENERATOR ZIP)
    
    set(INSTALL_DLLS
        ${CMAKE_INSTALL_PREFIX}/bin/avcodec-62.dll
        ${CMAKE_INSTALL_PREFIX}/bin/avdevice-62.dll
        ${CMAKE_INSTALL_PREFIX}/bin/avformat-62.dll
        ${CMAKE_INSTALL_PREFIX}/bin/avutil-60.dll
        ${CMAKE_INSTALL_PREFIX}/bin/swresample-6.dll
        ${CMAKE_INSTALL_PREFIX}/bin/swscale-9.dll)
    
    install(
        FILES ${INSTALL_DLLS}
        DESTINATION bin)

elseif(APPLE)

    set(CPACK_GENERATOR TGZ)

    list(APPEND CMAKE_INSTALL_RPATH
        "@executable_path/../lib")
    #set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    set(INSTALL_DYLIBS
        ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.62.11.100.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.62.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.62.1.100.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.62.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavformat.62.3.100.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavformat.62.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavformat.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavutil.60.8.100.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavutil.60.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libavutil.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libswresample.6.1.100.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libswresample.6.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libswresample.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libswscale.9.1.100.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libswscale.9.dylib
        ${CMAKE_INSTALL_PREFIX}/lib/libswscale.dylib)
        
    install(
        FILES ${INSTALL_DYLIBS}
        DESTINATION lib)

else()

    set(CPACK_GENERATOR TGZ)

    set(INSTALL_LIBS
        ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.so
        ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.so.62
        ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.so.62.11.100
        ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.so
        ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.so.62
        ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.so.62.1.100
        ${CMAKE_INSTALL_PREFIX}/lib/libavformat.so
        ${CMAKE_INSTALL_PREFIX}/lib/libavformat.so.62
        ${CMAKE_INSTALL_PREFIX}/lib/libavformat.so.62.3.100
        ${CMAKE_INSTALL_PREFIX}/lib/libavutil.so
        ${CMAKE_INSTALL_PREFIX}/lib/libavutil.so.60
        ${CMAKE_INSTALL_PREFIX}/lib/libavutil.so.60.8.100
        ${CMAKE_INSTALL_PREFIX}/lib/libswresample.so
        ${CMAKE_INSTALL_PREFIX}/lib/libswresample.so.6
        ${CMAKE_INSTALL_PREFIX}/lib/libswresample.so.6.1.100
        ${CMAKE_INSTALL_PREFIX}/lib/libswscale.so
        ${CMAKE_INSTALL_PREFIX}/lib/libswscale.so.9
        ${CMAKE_INSTALL_PREFIX}/lib/libswscale.so.9.1.100)
    
    install(
        FILES ${INSTALL_LIBS}
        DESTINATION lib)

endif()
