cmake_minimum_required(VERSION 2.8.6)
project(kinetic_cpp_client_examples CXX C)

find_package (Threads)

set(CMAKE_CXX_FLAGS "--std=c++11 -Wall -Wextra -Werror -Wno-unknown-warning-option -Wno-unused-parameter -Wno-unused-local-typedefs -Wno-null-dereference -DGTEST_USE_OWN_TR1_TUPLE=1 -D__STDC_FORMAT_MACROS")

set(GENERATED_SOURCES_PATH ${kinetic_cpp_client_SOURCE_DIR}/src/main/generated)

include(ExternalProject)

option(USE_LOCAL_KINETIC_CLIENT "Uses the kinetic client at the KINETIC_CLIENT_PATH variable instead of downloading library from Git" off)
if(USE_LOCAL_KINETIC_CLIENT)
else(USE_LOCAL_KINETIC_CLIENT)
    set(KINETIC_CLIENT_PATH "${CMAKE_BINARY_DIR}/vendor/src/kinetic_cpp_client")

    ExternalProject_add(
        kinetic_cpp_client
        PREFIX "vendor"
        GIT_REPOSITORY "https://github.com/Seagate/kinetic-cpp-client.git"
        GIT_TAG "0.0.3"
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""
    )
endif(USE_LOCAL_KINETIC_CLIENT)

include_directories(
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/vendor/include
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/include
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/vendor/src/gmock/fused-src
    ${KINETIC_CLIENT_PATH}/src/main/generated
    ${KINETIC_CLIENT_PATH}/include
)

set(LIBRARY_DEPENDENCIES
    ${KINETIC_CLIENT_PATH}/libkinetic_client.a
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/libprotobufutil.a
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/vendor/lib/libglog.a
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/vendor/lib/libgflags.a
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/vendor/lib/libssl.a
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/vendor/lib/libcrypto.a
    ${KINETIC_CLIENT_PATH}/vendor/src/protobufutil/vendor/lib/libprotobuf.a
    ${CMAKE_THREAD_LIBS_INIT}
    dl
)

macro(add_example_target NAME USE_EXAMPLE_MAIN)
    if(${USE_EXAMPLE_MAIN})
        add_executable(${NAME}
            src/example_main.cc
            src/${NAME}.cc)
    else(${USE_EXAMPLE_MAIN})
        add_executable(${NAME}
            src/${NAME}.cc)
    endif()

    target_link_libraries(${NAME} ${LIBRARY_DEPENDENCIES})
    add_dependencies(${NAME} kinetic_cpp_client)
endmacro(add_example_target)

add_example_target(kineticstat true)
add_example_target(ise true)
add_example_target(setclusterversion true)
add_example_target(setpin true)
add_example_target(set_acls true)
add_example_target(write_file_blocking true)
add_example_target(read_file_blocking true)
add_example_target(read_file_nonblocking false)
add_example_target(delete_file_nonblocking false)
add_example_target(delete_file_blocking false)
add_example_target(write_file_nonblocking true)
add_example_target(firmware_update false)
add_example_target(write_file_blocking_threads false)
add_example_target(dump_keyspace false)
add_example_target(copydrive false)
