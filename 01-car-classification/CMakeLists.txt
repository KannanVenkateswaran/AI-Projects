cmake_minimum_required(VERSION 3.16)
project(CarClassification LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

# Shared data loading and normalization.
add_library(car_data STATIC src/car_data.cpp)
target_include_directories(car_data PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Algorithm libraries and their application entry points.
add_library(data_analysis_lib STATIC src/data_analysis.cpp)
target_link_libraries(data_analysis_lib PUBLIC car_data)
add_executable(data_analysis src/data_analysis_main.cpp)
target_link_libraries(data_analysis PRIVATE data_analysis_lib)

add_library(linear_classifier_lib STATIC src/linear_classifier.cpp)
target_link_libraries(linear_classifier_lib PUBLIC car_data)
add_executable(linear_classifier src/linear_classifier_main.cpp)
target_link_libraries(linear_classifier PRIVATE linear_classifier_lib)

add_library(mcculloch_pitts_lib STATIC src/mcculloch_pitts.cpp)
target_include_directories(mcculloch_pitts_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_executable(mcculloch_pitts src/mcculloch_pitts_main.cpp)
target_link_libraries(mcculloch_pitts PRIVATE mcculloch_pitts_lib)

set(project_targets
    car_data
    data_analysis_lib data_analysis
    linear_classifier_lib linear_classifier
    mcculloch_pitts_lib mcculloch_pitts
)

if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    # Keep smoke-test inputs and outputs inside the build directory.
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data"
         DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

    foreach(program IN ITEMS data_analysis linear_classifier mcculloch_pitts)
        add_test(NAME ${program}_runs COMMAND $<TARGET_FILE:${program}>)
        set_tests_properties(${program}_runs PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endforeach()

    # The Python harness creates a temporary directory for each test.
    function(add_regression test_name executable_name)
        add_test(NAME ${test_name}
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression_tests.py"
                "${test_name}"
                "$<TARGET_FILE:${executable_name}>"
                "${CMAKE_CURRENT_SOURCE_DIR}/data"
        )
    endfunction()

    add_regression(classifier linear_classifier)
    add_regression(neuron mcculloch_pitts)
    add_regression(plots data_analysis)
    foreach(test_case IN ITEMS missing empty malformed constant invalid_label)
        add_regression(${test_case} data_analysis)
    endforeach()

    foreach(component IN ITEMS data_analysis linear_classifier mcculloch_pitts)
        add_executable(${component}_unit tests/test_${component}.cpp)
        target_link_libraries(${component}_unit PRIVATE ${component}_lib)
        add_test(NAME ${component}_unit COMMAND $<TARGET_FILE:${component}_unit>)
        list(APPEND project_targets ${component}_unit)
    endforeach()

    set(test_workspace "${CMAKE_CURRENT_BINARY_DIR}/data_analysis_unit_workspace")
    file(MAKE_DIRECTORY "${test_workspace}")
    set_tests_properties(data_analysis_unit PROPERTIES
        WORKING_DIRECTORY "${test_workspace}"
    )
endif()

# Apply warnings once to every declared C++ target, including unit tests.
foreach(target IN LISTS project_targets)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
