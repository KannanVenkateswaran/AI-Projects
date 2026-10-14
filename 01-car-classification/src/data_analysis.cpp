// CMSC 436 - Project 1 | Kannan Venkateswaran
// C++17 SVG visualization of independently normalized price and weight.
// Preserves the report's visualization line: x + y = threshold.
// Build and run instructions are in README.md.

#include "data_analysis.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

// Write a self-contained SVG using the C++ Library.
void plot_car_data(const Dataset& dataset, char group_name, double threshold,
                   const fs::path& output_file)
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

    const auto x_pixel = [](double normalized) { return left + normalized * plot_width; };
    const auto y_pixel = [](double normalized) { return top + (1.0 - normalized) * plot_height; };

    output << std::fixed << std::setprecision(3);
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\""
           << height << "\" viewBox=\"0 0 " << width << ' ' << height << "\">\n"
           << "<title>Group " << group_name << " - Normalized car data</title>\n"
           << "<desc>Normalized vehicle price versus weight, grouped by car class, "
              "with the original manually chosen separation line.</desc>\n"
           << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n"
           << "<text x=\"600\" y=\"65\" text-anchor=\"middle\" "
              "font-family=\"Arial,sans-serif\" font-size=\"26\" "
              "font-weight=\"bold\">Group "
           << group_name << " - Normalized car data</text>\n"
           << "<rect x=\"" << left << "\" y=\"" << top << "\" width=\"" << plot_width
           << "\" height=\"" << plot_height
           << "\" fill=\"none\" stroke=\"#555\" stroke-width=\"1.5\"/>\n";

    for (int tick = 0; tick <= 5; ++tick)
    {
        const double value = tick / 5.0;
        const double xp = x_pixel(value);
        const double yp = y_pixel(value);
        output << "<line x1=\"" << xp << "\" y1=\"" << top + plot_height << "\" x2=\"" << xp
               << "\" y2=\"" << top + plot_height + 9 << "\" stroke=\"#555\"/>\n"
               << "<text x=\"" << xp << "\" y=\"" << top + plot_height + 30
               << "\" text-anchor=\"middle\" font-family=\"Arial,sans-serif\" "
                  "font-size=\"17\">"
               << std::setprecision(1) << value << std::setprecision(3) << "</text>\n"
               << "<line x1=\"" << left - 9 << "\" y1=\"" << yp << "\" x2=\"" << left << "\" y2=\""
               << yp << "\" stroke=\"#555\"/>\n"
               << "<text x=\"" << left - 17 << "\" y=\"" << yp + 6
               << "\" text-anchor=\"end\" font-family=\"Arial,sans-serif\" "
                  "font-size=\"17\">"
               << std::setprecision(1) << value << std::setprecision(3) << "</text>\n";
    }

    output << "<text x=\"" << left + plot_width / 2
           << "\" y=\"850\" "
              "text-anchor=\"middle\" font-family=\"Arial,sans-serif\" "
              "font-size=\"21\">Normalized price</text>\n"
           << "<text transform=\"translate(43 " << top + plot_height / 2
           << ") rotate(-90)\" text-anchor=\"middle\" "
              "font-family=\"Arial,sans-serif\" font-size=\"21\">"
              "Normalized weight</text>\n";

    for (const Car& car : dataset.cars)
    {
        output << "<circle cx=\"" << x_pixel(car.normalized_price) << "\" cy=\""
               << y_pixel(car.normalized_weight) << "\" r=\"3.2\" fill=\""
               << (car.is_big_car == 0 ? "#2563EB" : "#EA580C") << "\" fill-opacity=\"0.55\"/>\n";
    }

    // The report used y = -x + T for plots. Clip the line to the plot box.
    const double x_start = std::max(0.0, threshold - 1.0);
    const double x_end = std::min(1.0, threshold);
    if (x_start <= x_end)
    {
        output << "<line x1=\"" << x_pixel(x_start) << "\" y1=\"" << y_pixel(threshold - x_start)
               << "\" x2=\"" << x_pixel(x_end) << "\" y2=\"" << y_pixel(threshold - x_end)
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
