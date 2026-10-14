cmake_minimum_required(VERSION 3.16)

project(CarClassification LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Build the three programs
foreach(program IN ITEMS 
    data_analysis
    linear_classifier
    mcculloch_pitts
)
    add_executable(${program} src/${program}.cpp)

    target_compile_options(${program} PRIVATE 
        -Wall
        -Wextra
        -Wpedantic
    )
endforeach()

#Enable automated testing
include(CTest)

if(BUILD_TESTING)

    #copy datasets into the build directory
    file(COPY "${CMAKE_SOURCE_DIR}/data"
        DESTINATION "${CMAKE_BINARY_DIR}")
    
    #Register one smoke test for each executable
    foreach(program IN ITEMS
        data_analysis
        linear_classifier
        mcculloch_pitts
    )

        add_test(
            NAME ${program}_runs
            COMMAND $<TARGET_FILE:${program}>
        )

        set_tests_properties(${program}_runs PROPERTIES
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )

    endforeach()

endif()

# Regression and edge-case tests

find_package(Python3 REQUIRED COMPONENTS Interpreter)

function(add_regression test_name executable_name)

    add_test(
        NAME ${test_name}
        COMMAND
            "${Python3_EXECUTABLE}"
            "${CMAKE_SOURCE_DIR}/tests/regression_tests.py"
            "${test_name}"
            "$<TARGET_FILE:${executable_name}>"
            "${CMAKE_SOURCE_DIR}/data"
    )

endfunction()


# Verify original classification results
add_regression(classifier linear_classifier)

# Verify the neuron and Boolean truth table
add_regression(neuron mcculloch_pitts)

# Verify normalization and SVG generation
add_regression(plots data_analysis)


# Verify invalid inputs are handled correctly
foreach(test_case IN ITEMS
    missing
    empty
    malformed
    constant
    invalid_label
)

    add_regression(${test_case} data_analysis)

endforeach()

# Native C++ unit tests

if(BUILD_TESTING)

    add_executable(
        mcculloch_pitts_unit
        tests/test_mcculloch_pitts.cpp
    )

    target_compile_definitions(
        mcculloch_pitts_unit
        PRIVATE CAR_CLASSIFICATION_UNIT_TEST
    )

    target_compile_options(
        mcculloch_pitts_unit PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(
        NAME mcculloch_pitts_unit
        COMMAND mcculloch_pitts_unit
    )

endif()

# Native C++ unit test for linear classification

if(BUILD_TESTING)

    add_executable(
        linear_classifier_unit
        tests/test_linear_classifier.cpp
    )

    target_compile_definitions(
        linear_classifier_unit
        PRIVATE CAR_CLASSIFICATION_UNIT_TEST
    )

    target_compile_options(
        linear_classifier_unit PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(
        NAME linear_classifier_unit
        COMMAND $<TARGET_FILE:linear_classifier_unit>
    )

endif()

# Native C++ unit tests for data analysis

if(BUILD_TESTING)

    add_executable(
        data_analysis_unit
        tests/test_data_analysis.cpp
    )

    target_compile_definitions(
        data_analysis_unit
        PRIVATE CAR_CLASSIFICATION_UNIT_TEST
    )

    target_compile_options(
        data_analysis_unit PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    # Give this test its own working directory.
    set(TEST_WORKSPACE
        "${CMAKE_CURRENT_BINARY_DIR}/data_analysis_unit_workspace"
    )

    file(MAKE_DIRECTORY "${TEST_WORKSPACE}")

    add_test(
        NAME data_analysis_unit
        COMMAND $<TARGET_FILE:data_analysis_unit>
    )

    set_tests_properties(
        data_analysis_unit PROPERTIES
        WORKING_DIRECTORY "${TEST_WORKSPACE}"
    )

endif()