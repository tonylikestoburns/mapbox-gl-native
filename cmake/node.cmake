# Load Node.js
include(node_modules/@mapbox/cmake-node-module/module.cmake)

add_node_module(mbgl-node
    INSTALL_DIR "${CMAKE_SOURCE_DIR}/lib"
    NAN_VERSION "2.8.0"
    EXCLUDE_NODE_ABIS 47 51 # Don't build old beta ABIs
)

target_sources(mbgl-node INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_mapbox_gl_native.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_mapbox_gl_native.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_logging.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_conversion.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_map.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_request.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_request.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_feature.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_feature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_thread_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_thread_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_expression.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/node_expression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/node/src/util/async_queue.hpp
)

target_include_directories(mbgl-node INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/default
)

target_link_libraries(mbgl-node INTERFACE
    mbgl-core
    mbgl-loop-uv
)

target_add_mason_package(mbgl-node INTERFACE geojson)

mbgl_platform_node()

create_source_groups(mbgl-node)

foreach(ABI IN LISTS mbgl-node::abis)
    initialize_xcode_cxx_build_settings(mbgl-node.abi-${ABI})
    xcode_create_scheme(
        TARGET mbgl-node.abi-${ABI}
        NAME "mbgl-node (ABI ${ABI})"
    )
endforeach()

xcode_create_scheme(
    TARGET mbgl-node.abi-${NodeJS_ABI}
    NAME "mbgl-node"
)

xcode_create_scheme(
    TARGET mbgl-node.all
    TYPE library
    NAME "mbgl-node (All ABIs)"
)

xcode_create_scheme(
    TARGET mbgl-node.abi-${NodeJS_ABI}
    TYPE node
    NAME "node tests"
    ARGS
        "node_modules/.bin/tape platform/node/test/js/**/*.test.js"
)

xcode_create_scheme(
    TARGET mbgl-node.abi-${NodeJS_ABI}
    TYPE node
    NAME "node render tests (ABI ${NodeJS_ABI}, v${NodeJS_VERSION})"
    ARGS
        "platform/node/test/render.test.js"
    OPTIONAL_ARGS
        "group"
        "test"
)

xcode_create_scheme(
    TARGET mbgl-node.abi-${NodeJS_ABI}
    TYPE node
    NAME "node query tests (ABI ${NodeJS_ABI}, v${NodeJS_VERSION})"
    ARGS
        "platform/node/test/query.test.js"
    OPTIONAL_ARGS
        "group"
        "test"
)

xcode_create_scheme(
    TARGET mbgl-node.abi-${NodeJS_ABI}
    TYPE node
    NAME "node expression tests (ABI ${NodeJS_ABI}, v${NodeJS_VERSION})"
    ARGS
        "platform/node/test/expression.test.js"
    OPTIONAL_ARGS
        "group"
        "test"
)

xcode_create_scheme(
    TARGET mbgl-node.abi-${NodeJS_ABI}
    TYPE node
    NAME "node-benchmark (ABI ${NodeJS_ABI}, v${NodeJS_VERSION})"
    ARGS
        "platform/node/test/benchmark.js"
    OPTIONAL_ARGS
        "group"
        "test"
)
