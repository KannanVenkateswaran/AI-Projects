/*
Name: Kannan Venkateswaran
Date: 10/12/2026
CMSC 436 - Project 1: McCulloch-Pitts Neuron (Part B)
File: src/mcculloch_pitts.cpp (C++17)

Compile from the project root:
    mkdir -p build
    clang++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/mcculloch_pitts.cpp -o build/mcculloch_pitts

Run:
    ./build/mcculloch_pitts

Reconstructs the original three-input neuron:
    weighted_sum = -3 * X + 3 * Y + Z
    output = 1 if weighted_sum >= -1; otherwise output = 0

The expected Boolean function is NOT X OR Y. We verify all eight binary
input combinations and determine the full valid threshold interval.
*/

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

// 1. Define the McCulloch-Pitts neuron's parameters and activation.
struct Neuron
{
    std::array<int, 3> weights{-3, 3, 1}; // Weights X, Y, and Z
    int threshold{-1};

    int weighted_sum(int x, int y, int z) const
    {
        return weights[0] * x + weights[1] * y + weights[2] * z;
    }
    int activate(int sum) const
    {
        return sum >= threshold ? 1: 0;
    }
};

// 2. Store a row of the truth table for display and export.
struct TruthRow
{
    int x{};
    int y{};
    int z{};
    int weighted_sum{};
    int neuron_output{};
    int expected_output{};
};

std::vector<TruthRow> make_truth_table(const Neuron& neuron)
{
    std::vector<TruthRow> rows;
    rows.reserve(8); // Three inputs = 2^3 = 8 combos;

    for (int x = 0; x <= 1; ++x)
    {
        for (int y = 0; y <= 1; ++y)
        {
            for (int z = 0; z <= 1; ++z)
            {
                const int sum = neuron.weighted_sum(x, y, z);
                const int output = neuron.activate(sum);
                const int expected = (x == 0 || y == 1) ? 1 : 0;

                //Ensuring our neuron implements NOT X OR Y on every row.
                if (output != expected)
                {
                    throw std::runtime_error("Neuron output does not match NOT X OR Y.");
                }

                rows.push_back({x, y, z, sum, output, expected});
            }
        }
    }
    return rows;
}

/*
3. Derive thresholds that produce exactly the expected truth table.
Rows expected to be 0 must have sum < T.
Rows expected to be 1 must have sum >= T.
Thus: max(sum for output 0) < T <= min(sum for output 1).
*/
struct ThresholdRange
{
    int lower_exclusive{};
    int upper_inclusive{};
};

ThresholdRange find_threshold_range(const std::vector<TruthRow>& rows)
{
    int lower = std::numeric_limits<int>::min();
    int upper = std::numeric_limits<int>::max();

    for (const TruthRow& row : rows)
    {
        if (row.expected_output == 0)
        {
            lower = std::max(lower, row.weighted_sum);
        }
        else
        {
            upper = std::min(upper, row.weighted_sum);
        }
    }

    if (lower >= upper)
    {
        throw std::runtime_error("No threshold matches the expected truth table.");
    }
    return {lower, upper};
}

//4. Print the truth table and save the results as CSV files.
void save_results(const Neuron& neuron, const std::vector<TruthRow>& rows, const ThresholdRange& range)
{
    fs::create_directories("results");

    std::ofstream truth_file("results/mcculloch_pitts_truth_table.csv");
    std::ofstream range_file("results/mcculloch_pitts_threshold_range.csv");
    if (!truth_file || !range_file)
    {
        throw std::runtime_error("Cannot open results CSV files for writing.");
    }

    truth_file << "X,Y,Z,weighted_sum,inequality,neuron_output,expected_not_X_or_Y\n";

    std::cout << "McCulloch-Pitts neuron: -3*X + 3*Y + Z >= -1\n"
              << "Boolean function: NOT X OR Y\n\n"
              << "X Y Z | Sum | Output | Expected\n"
              << "-------------------------------\n";
    
    for (const TruthRow& row : rows)
    {
        const char* comparison = row.neuron_output == 1 ? ">=" : "<";
        truth_file << row.x << ',' << row.y << ',' << row.z << ','
                   << row.weighted_sum << ',' << '"' << row.weighted_sum
                   << ' ' << comparison << ' ' << neuron.threshold << '"' << ','
                   << row.neuron_output << ',' << row.expected_output << '\n';

        std::cout << row.x << ' ' << row.y << ' ' << row.z
                  << " | " << std::setw(3) << row.weighted_sum
                  << " | " << std::setw(6) << row.neuron_output
                  << " | " << row.expected_output << '\n';
    }

    range_file << "weight_X,weight_Y,weight_Z,original_threshold,"
                  "lower_exclusive,upper_inclusive,logic_function\n"
               << neuron.weights[0] << ',' << neuron.weights[1] << ','
               << neuron.weights[2] << ',' << neuron.threshold << ','
               << range.lower_exclusive << ',' << range.upper_inclusive
               << ",NOT X OR Y\n";

    truth_file.close();
    range_file.close();
    if (!truth_file || !range_file)
    {
        throw std::runtime_error("Failed while writing results CSV files.");
    }

    std::cout << "\nVerification passed: all 8 rows match NOT X OR Y.\n"
              << "Valid threshold range: " << range.lower_exclusive
              << " < T <= " << range.upper_inclusive << '\n'
              << "Original threshold: T = " << neuron.threshold << " (valid)\n"
              << "\nSaved: results/mcculloch_pitts_truth_table.csv\n"
              << "Saved: results/mcculloch_pitts_threshold_range.csv\n"
              << "Done.\n";
}

// 5. Execute and handle errors.
int main()
{
    try
    {
        const Neuron neuron;
        const std::vector<TruthRow> rows = make_truth_table(neuron);
        const ThresholdRange range = find_threshold_range(rows);

        if (!(range.lower_exclusive < neuron.threshold && neuron.threshold <= range.upper_inclusive))
        {
            throw std::runtime_error("Original threshold is outside the valid range.");
        }

        save_results(neuron, rows, range);
        return 0;
    }
    catch(const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}