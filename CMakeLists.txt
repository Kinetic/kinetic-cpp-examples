cmake_minimum_required(VERSION 2.8.6)
project(kinetic_cpp_client_examples CXX C)

find_package (Threads)

set(CMAKE_CXX_FLAGS "-g --std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter -DGTEST_USE_OWN_TR1_TUPLE=1")

set(GENERATED_SOURCES_PATH ${kinetic_cpp_client_SOURCE_DIR}/src/main/generated)

include(ExternalProject)
ExternalProject_add(
    kinetic_cpp_client
    PREFIX "vendor"
    GIT_REPOSITORY "git@github.com:Seagate/Kinetic-C-Client.git"
    GIT_TAG "origin/master"
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

include_directories(
    vendor/src/kinetic_cpp_client/vendor/src/protobufutil/vendor/include
    vendor/src/kinetic_cpp_client/vendor/src/protobufutil/include
    vendor/src/kinetic_cpp_client/vendor/src/protobufutil/vendor/src/gmock/fused-src
    vendor/src/kinetic_cpp_client/src/main
    vendor/src/kinetic_cpp_client/src/main/generated
    vendor/src/kinetic_cpp_client/src/public_include
)

set(LIBRARY_DEPENDENCIES
    ${kinetic_cpp_client_examples_SOURCE_DIR}/vendor/src/kinetic_cpp_client/libkinetic_client_library.a
    ${kinetic_cpp_client_examples_SOURCE_DIR}/vendor/src/kinetic_cpp_client/vendor/src/protobufutil/libprotobufutil.a
    ${kinetic_cpp_client_examples_SOURCE_DIR}/vendor/src/kinetic_cpp_client/vendor/src/protobufutil/vendor/lib/libglog.a
    ${kinetic_cpp_client_examples_SOURCE_DIR}/vendor/src/kinetic_cpp_client/vendor/src/protobufutil/vendor/lib/libgflags.a
    ${kinetic_cpp_client_examples_SOURCE_DIR}/vendor/src/kinetic_cpp_client/vendor/src/protobufutil/vendor/lib/libssl.a
    ${kinetic_cpp_client_examples_SOURCE_DIR}/vendor/src/kinetic_cpp_client/vendor/src/protobufutil/vendor/lib/libcrypto.a
    ${kinetic_cpp_client_examples_SOURCE_DIR}/vendor/src/kinetic_cpp_client/vendor/src/protobufutil/vendor/lib/libprotobuf.a
    ${CMAKE_THREAD_LIBS_INIT}
    dl
)

macro(add_example_target NAME)
    add_executable(${NAME} src/${NAME}.cc)
    target_link_libraries(${NAME} ${LIBRARY_DEPENDENCIES})
    add_dependencies(${NAME} kinetic_cpp_client)
endmacro(add_example_target)

add_example_target(hello_kinetic)
add_example_target(kineticstat)
add_example_target(async)
add_example_target(ise)
add_example_target(setclusterversion)
add_example_target(setpin)
add_example_target(set_acls)
add_example_target(write_file_blocking)
add_example_target(read_file_blocking)
add_example_target(read_file_nonblocking)

