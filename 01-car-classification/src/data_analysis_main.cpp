// CMSC 436 - Project 1
// Data analysis application entry point.

#include "data_analysis.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

namespace fs = std::filesystem;

int main()
{
    try
    {
        // Thresholds used for the original scatter plots.
        const std::array<std::pair<char, double>, 3> groups{
            {{'A', 0.95}, {'B', 1.02}, {'C', 0.98}}};

        for (const auto& [name, threshold] : groups)
        {
            const fs::path input_file =
                fs::path("data") / ("group" + std::string(1, name) + ".txt");
            const Dataset dataset = load_car_data(input_file);

            const auto small_count =
                std::count_if(dataset.cars.begin(), dataset.cars.end(),
                              [](const Car& car) { return car.is_big_car == 0; });
            const fs::path output_file = fs::path("results") / "figures" /
                                         ("group" + std::string(1, name) + "_normalized.svg");

            std::cout << "\nGroup " << name << '\n'
                      << " Observations: " << dataset.cars.size() << '\n'
                      << " Small cars: " << small_count << '\n'
                      << " Big cars: " << (dataset.cars.size() - small_count) << '\n'
                      << " Price range (USD): " << dataset.prices.min << " to "
                      << dataset.prices.max << '\n'
                      << " Weight range (lbs): " << dataset.weights.min << " to "
                      << dataset.weights.max << '\n';

            plot_car_data(dataset, name, threshold, output_file);
            std::cout << " Saved plot: " << output_file.string() << '\n';
        }

        std::cout << "\nFinished! Open results/figures/ to view the SVG graphs.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
