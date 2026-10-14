/*
CMSC 436 - Project 1
Native C++ unit tests for the McCulloch-Pitts Neuron
*/

#include <array>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

/*
Include the implementation under test.
The original main() is excluded by a CMake compile definition
*/

#include "../src/mcculloch_pitts.cpp"

// Report a failed test rather than silently continuing.
void check(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error("Test failed: " + message);
    }
}

int main()
{
    try
    {
        // TEST 1: Original neuron parameters
        Neuron neuron;

        check(neuron.weights[0] == -3, "Weight X");
        check(neuron.weights[1] == 3, "Weight Y");
        check(neuron.weights[2] == 1, "Weight Z");
        check(neuron.threshold == -1, "Threshold");

        // TEST 2: Weighted sums
        check(neuron.weighted_sum(0, 0, 0) == 0,
              "Weighted sum for 000");

        check(neuron.weighted_sum(0, 1, 1) == 4,
              "Weighted sum for 011");

        check(neuron.weighted_sum(1, 0, 0) == -3,
              "Weighted sum for 100");

        check(neuron.weighted_sum(1, 1, 1) == 1,
              "Weighted sum for 111");

        // TEST 3: Threshold activation
        check(neuron.activate(-3) == 0,
              "Activation below threshold");

        check(neuron.activate(-2) == 0,
              "Activation immediately below threshold");

        check(neuron.activate(-1) == 1,
              "Activation at threshold");

        check(neuron.activate(0) == 1,
              "Activation above threshold");

        // TEST 4: Complete truth table
        const auto rows = make_truth_table(neuron);

        check(rows.size() == 8, "Truth table row count");

        const std::array<int, 8> expected_sums{
            0, 1, 3, 4, -3, -2, 0, 1
        };

        const std::array<int, 8> expected_outputs{
            1, 1, 1, 1, 0, 0, 1, 1
        };

        for (std::size_t i = 0; i < rows.size(); ++i)
        {
            check(rows[i].weighted_sum == expected_sums[i],
                  "Truth table weighted sum");

            check(rows[i].neuron_output == expected_outputs[i],
                  "Truth table output");

            // Verify input ordering: 000, 001, ..., 111.
            check(rows[i].x == static_cast<int>(i / 4) &&
                  rows[i].y == static_cast<int>((i / 2) % 2) &&
                  rows[i].z == static_cast<int>(i % 2),
                  "Truth table input ordering");
        }

        // TEST 5: Valid threshold range
        const auto range = find_threshold_range(rows);

        check(range.lower_exclusive == -2,
              "Lower threshold boundary");

        check(range.upper_inclusive == 0,
              "Upper threshold boundary");

        std::cout << "All McCulloch-Pitts unit tests passed.\n";

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
