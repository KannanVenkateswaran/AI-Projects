/*
Name: Kannan Venkateswaran
Date: 10/12/2026

CMSC 436 - Project 1: Car Classification
File: linear_classifier.cpp (C++17) - refactor of linear_classifier.R reconstruction

Compile from the project root:
    mkdir -p build
    clang++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/linear_classifier.cpp -o build/linear_classifier
    ./build/linear_classifier

Run:
    ./build/linear_classifier

Reconstructs the classification calculations from the original report:
    prediction = 1 if 2 * normalized_price + normalized_weight >= threshold
    prediction = 0 otherwise.

The original scatterplots use a different equation: x + y = threshold.
Both formulas are preserved in their respective programs.
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
#include <utility>
#include <vector>

namespace fs = std::filesystem;

// 1. Represent cars and normalization ranges
struct Car
{
    double price_usd{};
    double weight_lbs{};
    int is_big_car{}; // Actual label: 0 = small, 1 = big
    double normalized_price{};
    double normalized_weight{};
};

struct Range
{
    double min = std::numeric_limits<double>::infinity();
    double max = -std::numeric_limits<double>::infinity();

    double normalize(double value) const
    {
        if (min == max)
        {
            throw std::runtime_error("Cannot normalize a constant-value column.");
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

// 2. Load and normalize each original dataset
double parse_number(const std::string& text)
{
    std::size_t consumed = 0;
    const double value = std::stod(text, & consumed);
    if (text.find_first_not_of(" \t\r\n", consumed) != std::string::npos || !std::isfinite(value))
    {
        throw std::invalid_argument("Invalid numeric value: " + text);
    }
    
    return value;
}

Dataset load_car_data(const fs::path& input_file)
{
    std::ifstream input(input_file);
    if (!input)
    {
        throw std::runtime_error("Cannot open " + input_file.string() + ". Run this program from the project root.");
    }

    Dataset dataset;
    std::string line;
    std::size_t line_number = 0;

    while (std::getline(input, line))
    {
        ++line_number;
        if (line.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            continue;
        }

        try
        {
            // Headerless CSV: price_usd, weight_lbs, is_big_car
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
        throw std::runtime_error("Dataset is empty:" + input_file.string());
    }

    for (Car& car : dataset.cars)
    {
        car.normalized_price = dataset.prices.normalize(car.price_usd);
        car.normalized_weight = dataset.weights.normalize(car.weight_lbs);
    }

    return dataset;
}

// 3. Predict the car class
int predict_car_size(const Car& car, double threshold)
{
    const double weighted_sum = 2.0 * car.normalized_price + car.normalized_weight;
    return weighted_sum >= threshold ? 1 : 0;
}

// 4. Evaluate predictions against the actual labels
struct ConfusionMatrix
{
    std::size_t tp{}; // Actual big, predicted big
    std::size_t fp{}; // Actual small, predicted big
    std::size_t fn{}; // Actual big, predicted small
    std::size_t tn{}; // Actual small, predicted small
};

ConfusionMatrix evaluate(const Dataset& dataset, double threshold)
{
    ConfusionMatrix cm;
    for (const Car& car : dataset.cars)
    {
        const int prediction = predict_car_size(car, threshold);
        if (car.is_big_car == 1 && prediction == 1) ++cm.tp;
        else if (car.is_big_car == 0 && prediction == 1) ++cm.fp;
        else if (car.is_big_car == 1 && prediction == 0) ++cm.fn;
        else ++cm.tn;
    }

    return cm;
}

//Return NaN when the metric's denominator is zero (undefined metric).
double percent(double numerator, double denominator)
{
    if (denominator == 0.0)
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return 100.0 * numerator / denominator;
}

struct Metrics
{
    double accuracy{};
    double error{};
    double precision{};
    double recall{};
    double specificity{};
    double false_positive_rate{};
    double false_negative_rate{};
    double f1{};
};

Metrics calculate_metrics(const ConfusionMatrix& cm)
{
    const double tp = static_cast<double>(cm.tp);
    const double fp = static_cast<double>(cm.fp);
    const double fn = static_cast<double>(cm.fn);
    const double tn = static_cast<double>(cm.tn);
    const double total = tp + fp + fn + tn;

    return
    {
        percent(tp + tn, total),       // Accuracy
        percent(fp + fn, total),       // Error
        percent(tp, tp + fp),         // Precision
        percent(tp, tp + fn),         // Recall (true positive rate)
        percent(tn, tn + fp),         // Specificity
        percent(fp, fp + tn),         // False positive rate
        percent(fn, fn + tp),         // False negative rate
        percent(2 * tp, 2 * tp + fp + fn) // F1-score
    };
}

// 5. Print results and save two CSV files
int main()
{
    try
    {
        const std::array<std::pair<char, double>, 3> groups {{{'A', 0.95}, {'B', 1.02}, {'C', 0.98}}};

        fs::create_directories("results");
        std::ofstream metrics_file("results/classification_metrics.csv");
        std::ofstream matrix_file("results/confusion_matrices.csv");
        if (!metrics_file || !matrix_file)
        {
            throw std::runtime_error("Could not open results CSV files for writing.");
        }

        metrics_file << "group,threshold,observations,TP,FP,FN,TN,"
                        "accuracy_pct,error_pct,precision_pct,recall_pct,"
                        "specificity_pct,false_positive_rate_pct,"
                        "false_negative_rate_pct,f1_pct\n";
        matrix_file << "group,actual,predicted,count\n";
        metrics_file << std::fixed <<std::setprecision(2);
        std::cout << std::fixed << std::setprecision(2);

        for (const auto& [name, threshold] : groups)
        {
            const fs::path input_file = fs::path("data") / ("group" + std::string(1, name) + ".txt");
            const Dataset dataset = load_car_data(input_file);
            const ConfusionMatrix cm = evaluate(dataset, threshold);
            const Metrics m = calculate_metrics(cm);

            std::cout << "\nGroup " << name << " (threshold = " << threshold << ")\n"
                      << "                  Predicted 0   Predicted 1\n"
                      << "  Actual 0       " << cm.tn << "          " << cm.fp << '\n'
                      << "  Actual 1       " << cm.fn << "          " << cm.tp << '\n'
                      << "  Accuracy: " << m.accuracy << "% | Precision: "
                      << m.precision << "% | Recall: " << m.recall
                      << "% | F1: " << m.f1 << "%\n";

            metrics_file << name << ',' << threshold << ',' << dataset.cars.size()
                         << ',' << cm.tp << ',' << cm.fp << ',' << cm.fn
                         << ',' << cm.tn << ',' << m.accuracy << ',' << m.error
                         << ',' << m.precision << ',' << m.recall
                         << ',' << m.specificity << ',' << m.false_positive_rate
                         << ',' << m.false_negative_rate << ',' << m.f1 << '\n';

            // Rows = actual label, columns = predicted label.
            matrix_file << name << ",0,0," << cm.tn << '\n'
                        << name << ",0,1," << cm.fp << '\n'
                        << name << ",1,0," << cm.fn << '\n'
                        << name << ",1,1," << cm.tp << '\n';
        }

        metrics_file.close();
        matrix_file.close();
        if (!metrics_file || !matrix_file)
        {
            throw std::runtime_error("Failed while writing results CSV files.");
        }

        std::cout << "\nSaved: results/classification_metrics.csv\n"
                  << "Saved: results/confusion_matrices.csv\n"
                  << "Done.\n";

            return 0;
    }
    catch(const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}