#ifndef CAR_DATA_HPP
#define CAR_DATA_HPP

#include <filesystem>
#include <limits>
#include <string>
#include <vector>

// Represents a single vehicle observation.
struct Car
{
    double price_usd{};
    double weight_lbs{};
    int is_big_car{};

    double normalized_price{};
    double normalized_weight{};
};

// Stores the minimum and maximum values of a feature.
struct Range
{
    double min = std::numeric_limits<double>::infinity();
    double max = -std::numeric_limits<double>::infinity();

    double normalize(double value) const;
};

// Represents a dataset containing vehicle observations.
struct Dataset
{
    std::vector<Car> cars;

    Range prices;
    Range weights;
};

// Converts a string into a validated numeric value.
double parse_number(const std::string& text);

// Loads and normalizes a vehicle dataset.
Dataset load_car_data(const std::filesystem::path& input_file);

#endif
