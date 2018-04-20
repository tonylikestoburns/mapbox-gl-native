add_library(mbgl-loop-darwin INTERFACE)

target_sources(mbgl-loop-darwin INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/darwin/src/async_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/darwin/src/run_loop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/darwin/src/timer.cpp
)

target_include_directories(mbgl-loop-darwin INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

create_source_groups(mbgl-loop-darwin)
