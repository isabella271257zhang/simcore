cmake_minimum_required(VERSION 3.20)

project(
    simcore
    VERSION 0.1.0
    DESCRIPTION "Reusable discrete-time simulation core library"
    LANGUAGES CXX)

add_library(simcore STATIC src/stepper.cpp)
add_library(simcore::simcore ALIAS simcore)

target_include_directories(
    simcore
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>)

target_compile_features(simcore PUBLIC cxx_std_20)

set_target_properties(simcore PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simcore PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Examples and tests belong to this repository, not to consumers that pull the
# library in with add_subdirectory().
option(SIMCORE_BUILD_TESTS "Build the simcore unit tests" ON)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(examples)

    if(SIMCORE_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()

    find_package(Doxygen 1.9 QUIET)

    if(DOXYGEN_FOUND)
        add_custom_target(
            doc
            COMMAND
                ${CMAKE_COMMAND} -E env
                SIMCORE_DOC_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/docs
                ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating simcore API documentation"
            VERBATIM)
    else()
        add_custom_target(
            doc
            COMMAND
                ${CMAKE_COMMAND} -E echo
                "Doxygen was not found at configure time. Install Doxygen and reconfigure, then rerun: cmake --build <build> --target doc"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "Doxygen is required to generate the API documentation"
            VERBATIM)
    endif()
endif()
