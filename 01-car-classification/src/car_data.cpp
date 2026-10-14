// CMSC 436 - Project 1: Car Classification
// Shared dataset loading and normalization implementation.

#include "car_data.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

// Normalize a value using min-max normalization.

double Range::normalize(double value) const
{
    if (min == max)
    {
        throw std::runtime_error("Unable to normalize a column with identical values.");
    }

    return (value - min) / (max - min);
}

// Convert a string into a validated double.

double parse_number(const std::string& text)
{
    std::size_t consumed = 0;

    const double value = std::stod(text, &consumed);

    if (text.find_first_not_of(" \t\r\n", consumed) != std::string::npos || !std::isfinite(value))
    {
        throw std::invalid_argument("Invalid numeric value: " + text);
    }

    return value;
}

// Load and normalize a vehicle dataset.

Dataset load_car_data(const fs::path& input_file)
{
    std::ifstream input(input_file);

    if (!input)
    {
        throw std::runtime_error("Cannot open " + input_file.string() +
                                 ". Run this program from the project's root directory.");
    }

    Dataset dataset;

    std::string line;
    std::size_t line_number = 0;

    while (std::getline(input, line))
    {
        ++line_number;

        // Skip empty or whitespace-only lines.
        if (line.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            continue;
        }

        try
        {
            // Original format:
            // price_usd,weight_lbs,is_big_car

            if (std::count(line.begin(), line.end(), ',') != 2)
            {
                throw std::invalid_argument("Expected exactly three CSV fields.");
            }

            std::istringstream row(line);

            std::string price_text;
            std::string weight_text;
            std::string label_text;

            std::getline(row, price_text, ',');
            std::getline(row, weight_text, ',');
            std::getline(row, label_text, ',');

            Car car;

            car.price_usd = parse_number(price_text);
            car.weight_lbs = parse_number(weight_text);

            const double label = parse_number(label_text);

            if (label != 0.0 && label != 1.0)
            {
                throw std::invalid_argument("Class label must be 0 or 1.");
            }

            car.is_big_car = static_cast<int>(label);

            // Update feature ranges.
            dataset.prices.min = std::min(dataset.prices.min, car.price_usd);

            dataset.prices.max = std::max(dataset.prices.max, car.price_usd);

            dataset.weights.min = std::min(dataset.weights.min, car.weight_lbs);

            dataset.weights.max = std::max(dataset.weights.max, car.weight_lbs);

            // Store the observation.
            dataset.cars.push_back(car);
        }
        catch (const std::exception& error)
        {
            throw std::runtime_error(input_file.string() + ":" + std::to_string(line_number) +
                                     ": " + error.what());
        }
    }

    // Verify that reading succeeded.
    if (input.bad())
    {
        throw std::runtime_error("Error reading " + input_file.string());
    }

    if (dataset.cars.empty())
    {
        throw std::runtime_error("Dataset is empty: " + input_file.string());
    }

    // Normalize both features independently.
    for (Car& car : dataset.cars)
    {
        car.normalized_price = dataset.prices.normalize(car.price_usd);

        car.normalized_weight = dataset.weights.normalize(car.weight_lbs);
    }

    return dataset;
}
