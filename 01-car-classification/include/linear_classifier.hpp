#ifndef LINEAR_CLASSIFIER_HPP
#define LINEAR_CLASSIFIER_HPP

#include "car_data.hpp"

#include <cstddef>

// Predict whether a car is small or large.
int predict_car_size(const Car& car, double threshold);

// Store classification outcomes.
struct ConfusionMatrix
{
    std::size_t tp{};
    std::size_t fp{};
    std::size_t fn{};
    std::size_t tn{};
};

// Evaluate predictions against actual labels.
ConfusionMatrix evaluate(const Dataset& dataset, double threshold);

// Convert a ratio into a percentage.
double percent(double numerator, double denominator);

// Store classification performance measurements.
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

// Calculate performance metrics.
Metrics calculate_metrics(const ConfusionMatrix& cm);

#endif
