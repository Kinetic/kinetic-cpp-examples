cmake_minimum_required(VERSION 2.8.6)
project(kinetic_cpp_client_examples CXX C)

find_package (Threads)

set(CMAKE_CXX_FLAGS "-g --std=c++11 -Wall -Wextra -Werror -Wno-unknown-warning-option -Wno-unused-parameter -Wno-unused-local-typedefs -Wno-null-dereference -DGTEST_USE_OWN_TR1_TUPLE=1 -D__STDC_FORMAT_MACROS")

set(GENERATED_SOURCES_PATH ${kinetic_cpp_client_SOURCE_DIR}/src/main/generated)

include(ExternalProject)

option(USE_LOCAL_KINETIC_CLIENT "Uses the kinetic client at the KINETIC_CLIENT_PATH variable instead of downloading library from Git" off)
if(USE_LOCAL_KINETIC_CLIENT)
else(USE_LOCAL_KINETIC_CLIENT)
    set(KINETIC_CLIENT_PATH "${kinetic_cpp_client_examples_SOURCE_DIR}/vendor/src/kinetic_cpp_client")

    ExternalProject_add(
        kinetic_cpp_client
        PREFIX "vendor"
        GIT_REPOSITORY "git@github.com:Seagate/Kinetic-C-Client.git"
        GIT_TAG "f774a6e15a19df10d5593f763d64fe6b6f27383a"
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

macro(add_example_target NAME)
    add_executable(${NAME} src/${NAME}.cc)
    target_link_libraries(${NAME} ${LIBRARY_DEPENDENCIES})
    add_dependencies(${NAME} kinetic_cpp_client)

    set_target_properties( ${NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}
    )
endmacro(add_example_target)

add_example_target(kineticstat)
add_example_target(ise)
add_example_target(setclusterversion)
add_example_target(setpin)
add_example_target(set_acls)
add_example_target(write_file_blocking)
add_example_target(read_file_blocking)
add_example_target(read_file_nonblocking)
add_example_target(delete_file_nonblocking)
add_example_target(delete_file_blocking)
add_example_target(write_file_nonblocking)
add_example_target(firmware_update)
add_example_target(write_file_blocking_threads)
add_example_target(dump_keyspace)
