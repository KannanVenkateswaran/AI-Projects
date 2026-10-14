#ifndef DATA_ANALYSIS_HPP
#define DATA_ANALYSIS_HPP

#include "car_data.hpp"

#include <filesystem>

// Generate an SVG visualization of normalized vehicle data.
void plot_car_data(const Dataset& dataset, char group_name, double threshold,
                   const std::filesystem::path& output_file);

#endif
