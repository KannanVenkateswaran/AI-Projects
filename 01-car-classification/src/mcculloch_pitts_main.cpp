// CMSC 436 - Project 1
// McCulloch-Pitts application entry point.

#include "mcculloch_pitts.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>

int main()
{
    try
    {
        const Neuron neuron;

        const std::vector<TruthRow> rows = make_truth_table(neuron);

        const ThresholdRange range = find_threshold_range(rows);

        // Verify that the original threshold is valid.

        if (!(range.lower_exclusive < neuron.threshold &&
              neuron.threshold <= range.upper_inclusive))
        {
            throw std::runtime_error("Original threshold is outside the valid range.");
        }

        // Print the results and export the CSV files.

        save_results(neuron, rows, range);

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';

        return 1;
    }
}
