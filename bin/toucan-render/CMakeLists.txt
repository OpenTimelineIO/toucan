set(HEADERS
    App.h)
set(SOURCE
    App.cpp
    main.cpp)

add_executable(toucan-render ${HEADERS} ${SOURCE})
target_link_libraries(toucan-render toucanRender)
set_target_properties(toucan-render PROPERTIES FOLDER bin)
add_dependencies(toucan-render ${TOUCAN_PLUGINS})

install(
    TARGETS toucan-render
    RUNTIME DESTINATION bin)

foreach(OTIO CompositeTracks Draw Filter Gap Generator LinearTimeWarp Transition Transition2 Transform)
    add_test(
        toucan-render-${OTIO}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/toucan-render${CMAKE_EXECUTABLE_SUFFIX}
        ${PROJECT_SOURCE_DIR}/data/${OTIO}.otio ${OTIO}.png)
endforeach()

