/*
Name: Kannan Venkateswaran
Date: 10/12/2026

CMSC 436 - Project 1: Car Classification
File: data_analysis.cpp (C++17) - refactor of data.analysis.R reconstruction

Run from the project root:
    mkdir -p build
    clang++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/data_analysis.cpp -o build/data_analysis
    .build/data_analysis

This program laods the original data, normalizes price and weight to [0, 1] independently for each group,
and generates SVG (Scalable Vector Graphics, not Shane Van Gisbergen) scatter plots.
It preserves the report's visualization lines (x + y = threshold).
Classification predictions and metrics are belonging in the linear_classifier.cpp file, which is a separate module.
*/

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

namespace fs = std::filesystem;

struct Car
{
    double price_usd{};
    double weight_lbs{};
    int is_big_car{}; // 0 = small car, 1 = big car
    double normalized_price{};
    double normalized_weight{};
};

struct Range
{
    double min{};
    double max{};

    double normalize(double value) const
    {
        if (max == min)
        {
            throw std::runtime_error("Unable to normalize a column with identical values.");
        }
        return (value - min) / (max - min);
    }
};

struct Dataset
{
    std::vector<Car> cars;
    Range prices;
    Range weights;
};

//Reject malformed values instead of accepting partial numbers.
double parse_number(const std::string& text)
{
    std::size_t consumed = 0;
    const double value = std::stod(text, &consumed);
    if (text.find_first_not_of(" \t\r\n", consumed) != std::string::npos || !std::isfinite(value))
    {
        throw std::invalid_argument("Invalid numberic value: " + text);
    }
    return value;
}

Dataset load_car_data(const fs::path& input_file)
{
    std::ifstream input(input_file);
    if (!input)
    {
        throw std::runtime_error("Cannot open " + input_file.string() + ". Run this program from the project's root directory;.");
    };


    Dataset dataset;
    dataset.prices = {std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
    dataset.weights = {std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
    
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line))
    {
        ++line_number;
        if (line.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            continue; // skip empty lines
        }
        
        //Original .txt format: price_usd, weight_lbs, is_big_car (no header).
        try
        {
            if (std::count(line.begin(), line.end(), ',') != 2)
            {
                throw std::invalid_argument("Expected exactly three CSV fields.");
            }
            
            std::istringstream row(line);
            std::string price_text, weight_text, label_text;
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
            dataset.prices.min = std::min(dataset.prices.min, car.price_usd);
            dataset.prices.max = std::max(dataset.prices.max, car.price_usd);
            dataset.weights.min = std::min(dataset.weights.min, car.weight_lbs);
            dataset.weights.max = std::max(dataset.weights.max, car.weight_lbs);
            dataset.cars.push_back(car);
        }
        catch (const std::exception& error)
        {
            throw std::runtime_error(input_file.string() + ":" + std::to_string(line_number) + ":" + error.what());
        }
    }

    if (input.bad())
    {
        throw std::runtime_error("Error reading " + input_file.string());
    }

    if (dataset.cars.empty())
    {
        throw std::runtime_error("Dataset is empty: " + input_file.string());
    }

    for (Car& car : dataset.cars)
    {
        car.normalized_price = dataset.prices.normalize(car.price_usd);
        car.normalized_weight = dataset.weights.normalize(car.weight_lbs);
    }

    return dataset;
}

//Write a self-contained SVG using the C++ Library.
void plot_car_data(const Dataset & dataset, char group_name, double threshold, const fs::path& output_file)
{
    fs::create_directories(output_file.parent_path());
    std::ofstream output(output_file);
    if (!output)
    {
        throw std::runtime_error("Cannot write " + output_file.string());
    }

    constexpr int width = 1200;
    constexpr int height = 900;
    constexpr double left = 125;
    constexpr double top = 120;
    constexpr double plot_width = 940;
    constexpr double plot_height = 660;

    const auto x_pixel = [](double normalized)
    {
        return left + normalized * plot_width;
    };
    const auto y_pixel = [](double normalized)
    {
        return top + (1.0 - normalized) * plot_height;
    };

    output << std::fixed << std::setprecision(3);
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width
           << "\" height=\"" << height << "\" viewBox=\"0 0 " << width
           << ' ' << height << "\">\n"
           << "<title>Group " << group_name << " - Normalized car data</title>\n"
           << "<desc>Normalized vehicle price versus weight, grouped by car class, "
              "with the original manually chosen separation line.</desc>\n"
           << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n"
           << "<text x=\"600\" y=\"65\" text-anchor=\"middle\" "
              "font-family=\"Arial,sans-serif\" font-size=\"26\" "
              "font-weight=\"bold\">Group " << group_name
           << " - Normalized car data</text>\n"
           << "<rect x=\"" << left << "\" y=\"" << top
           << "\" width=\"" << plot_width << "\" height=\"" << plot_height
           << "\" fill=\"none\" stroke=\"#555\" stroke-width=\"1.5\"/>\n";

    for (int tick = 0; tick <= 5; ++tick)
    {
        const double value = tick / 5.0;
        const double xp = x_pixel(value);
        const double yp = y_pixel(value);
        output << "<line x1=\"" << xp << "\" y1=\"" << top + plot_height
               << "\" x2=\"" << xp << "\" y2=\"" << top + plot_height + 9
               << "\" stroke=\"#555\"/>\n"
               << "<text x=\"" << xp << "\" y=\"" << top + plot_height + 30
               << "\" text-anchor=\"middle\" font-family=\"Arial,sans-serif\" "
                  "font-size=\"17\">" << std::setprecision(1) << value
               << std::setprecision(3) << "</text>\n"
               << "<line x1=\"" << left - 9 << "\" y1=\"" << yp
               << "\" x2=\"" << left << "\" y2=\"" << yp
               << "\" stroke=\"#555\"/>\n"
               << "<text x=\"" << left - 17 << "\" y=\"" << yp + 6
               << "\" text-anchor=\"end\" font-family=\"Arial,sans-serif\" "
                  "font-size=\"17\">" << std::setprecision(1) << value
               << std::setprecision(3) << "</text>\n";
    }

    output << "<text x=\"" << left + plot_width / 2 << "\" y=\"850\" "
              "text-anchor=\"middle\" font-family=\"Arial,sans-serif\" "
              "font-size=\"21\">Normalized price</text>\n"
           << "<text transform=\"translate(43 " << top + plot_height / 2
           << ") rotate(-90)\" text-anchor=\"middle\" "
              "font-family=\"Arial,sans-serif\" font-size=\"21\">"
              "Normalized weight</text>\n";

    for (const Car& car : dataset.cars) 
    {
        output << "<circle cx=\"" << x_pixel(car.normalized_price)
               << "\" cy=\"" << y_pixel(car.normalized_weight)
               << "\" r=\"3.2\" fill=\""
               << (car.is_big_car == 0 ? "#2563EB" : "#EA580C")
               << "\" fill-opacity=\"0.55\"/>\n";
    }

    // The report used y = -x + T for plots. Clip the line to the plot box.
    const double x_start = std::max(0.0, threshold - 1.0);
    const double x_end = std::min(1.0, threshold);
    if (x_start <= x_end) 
    {
        output << "<line x1=\"" << x_pixel(x_start)
               << "\" y1=\"" << y_pixel(threshold - x_start)
               << "\" x2=\"" << x_pixel(x_end)
               << "\" y2=\"" << y_pixel(threshold - x_end)
               << "\" stroke=\"#111827\" stroke-width=\"2.5\"/>\n";
    }

    // Legend background makes the labels readable over the data points.
    output << "<rect x=\"145\" y=\"137\" width=\"310\" height=\"108\" "
              "fill=\"white\" fill-opacity=\"0.92\" stroke=\"#ddd\"/>\n"
           << "<circle cx=\"167\" cy=\"164\" r=\"5\" fill=\"#2563EB\"/>\n"
           << "<circle cx=\"167\" cy=\"197\" r=\"5\" fill=\"#EA580C\"/>\n"
           << "<line x1=\"156\" y1=\"227\" x2=\"180\" y2=\"227\" "
              "stroke=\"#111827\" stroke-width=\"2.5\"/>\n"
           << "<g font-family=\"Arial,sans-serif\" font-size=\"18\" fill=\"#111827\">\n"
           << "<text x=\"190\" y=\"170\">Small car (0)</text>\n"
           << "<text x=\"190\" y=\"203\">Big car (1)</text>\n"
           << "<text x=\"190\" y=\"233\">Original separation line</text>\n"
           << "</g>\n</svg>\n";

    if (!output) 
    {
        throw std::runtime_error("Error writing " + output_file.string());
    }
}

int main()
{
    try
    {
        //Thresholds used for the original scatter plots.
        const std::array<std::pair<char, double>, 3> groups{{{'A', 0.95}, {'B', 1.02}, {'C', 0.98}}};

    for (const auto& [name, threshold] : groups)
    {
        const fs::path input_file = fs::path("data") / ("group" + std::string(1, name) + ".txt");
        const Dataset dataset = load_car_data(input_file);

        const auto small_count = std::count_if(dataset.cars.begin(), dataset.cars.end(), [](const Car& car) { return car.is_big_car == 0; });
        const fs::path output_file = fs::path("results") / "figures" / ("group" + std::string(1, name) +"_normalized.svg");

        std::cout << "\nGroup " << name << '\n'
                    << " Observations: " << dataset.cars.size() << '\n'
                    << " Small cars: " << small_count << '\n'
                    << " Big cars: " << (dataset.cars.size() - small_count) << '\n'
                    << " Price range (USD): " << dataset.prices.min << " to " << dataset.prices.max << '\n'
                    << " Weight range (lbs): " << dataset.weights.min << " to " << dataset.weights.max << '\n';

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