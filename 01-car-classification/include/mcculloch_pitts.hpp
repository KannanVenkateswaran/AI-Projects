#ifndef MCCULLOCH_PITTS_HPP
#define MCCULLOCH_PITTS_HPP

#include <array>
#include <vector>

// Define the artificial neuron.

struct Neuron
{
    std::array<int, 3> weights{-3, 3, 1};
    int threshold{-1};

    int weighted_sum(int x, int y, int z) const;

    int activate(int sum) const;
};

// Represent one truth-table observation.

struct TruthRow
{
    int x{};
    int y{};
    int z{};

    int weighted_sum{};
    int neuron_output{};
    int expected_output{};
};

// Represent the valid threshold interval.

struct ThresholdRange
{
    int lower_exclusive{};
    int upper_inclusive{};
};

// Declare the neuron algorithms.

std::vector<TruthRow> make_truth_table(const Neuron& neuron);

ThresholdRange find_threshold_range(const std::vector<TruthRow>& rows);

void save_results(const Neuron& neuron, const std::vector<TruthRow>& rows,
                  const ThresholdRange& range);

#endif
