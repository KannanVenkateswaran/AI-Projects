// CMSC 436 - Project 1 | Kannan Venkateswaran
// Fixed weighted linear threshold classifier: 2*x + y >= threshold.
// The visualization uses a different equation, x + y = threshold.
// Both formulas preserve the original report. See README.md for usage.

#include "linear_classifier.hpp"

#include <limits>

// Predict the car class
int predict_car_size(const Car& car, double threshold)
{
    const double weighted_sum = 2.0 * car.normalized_price + car.normalized_weight;
    return weighted_sum >= threshold ? 1 : 0;
}

ConfusionMatrix evaluate(const Dataset& dataset, double threshold)
{
    ConfusionMatrix cm;
    for (const Car& car : dataset.cars)
    {
        const int prediction = predict_car_size(car, threshold);
        if (car.is_big_car == 1 && prediction == 1)
            ++cm.tp;
        else if (car.is_big_car == 0 && prediction == 1)
            ++cm.fp;
        else if (car.is_big_car == 1 && prediction == 0)
            ++cm.fn;
        else
            ++cm.tn;
    }

    return cm;
}

// Return NaN when the metric's denominator is zero (undefined metric).
double percent(double numerator, double denominator)
{
    if (denominator == 0.0)
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return 100.0 * numerator / denominator;
}

Metrics calculate_metrics(const ConfusionMatrix& cm)
{
    const double tp = static_cast<double>(cm.tp);
    const double fp = static_cast<double>(cm.fp);
    const double fn = static_cast<double>(cm.fn);
    const double tn = static_cast<double>(cm.tn);
    const double total = tp + fp + fn + tn;

    return {
        percent(tp + tn, total),          // Accuracy
        percent(fp + fn, total),          // Error
        percent(tp, tp + fp),             // Precision
        percent(tp, tp + fn),             // Recall (true positive rate)
        percent(tn, tn + fp),             // Specificity
        percent(fp, fp + tn),             // False positive rate
        percent(fn, fn + tp),             // False negative rate
        percent(2 * tp, 2 * tp + fp + fn) // F1-score
    };
}
