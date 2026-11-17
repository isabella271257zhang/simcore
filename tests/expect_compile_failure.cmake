# Builds one target and requires the build to fail with the diagnostic emitted by
# the simcore::Entity concept check. Registered as a ctest test; parameters are
# passed with -D from tests/CMakeLists.txt.

foreach(required BUILD_DIR TARGET)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "expect_compile_failure.cmake: ${required} is not defined")
    endif()
endforeach()

set(build_command "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --target "${TARGET}")
if(DEFINED CONFIG AND NOT CONFIG STREQUAL "")
    list(APPEND build_command --config "${CONFIG}")
endif()

execute_process(
    COMMAND ${build_command}
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_stdout
    ERROR_VARIABLE build_stderr)

if(build_result EQUAL 0)
    message(FATAL_ERROR "Expected target '${TARGET}' to fail compilation, but it succeeded.")
endif()

set(diagnostics "${build_stdout}\n${build_stderr}")
if(NOT diagnostics MATCHES "does not satisfy the simcore::Entity concept")
    message(FATAL_ERROR
            "Target '${TARGET}' failed without the expected simcore::Entity diagnostic:\n"
            "${diagnostics}")
endif()

message(STATUS "Target '${TARGET}' was rejected with the expected simcore::Entity diagnostic.")
