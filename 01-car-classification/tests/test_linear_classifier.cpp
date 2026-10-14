// CMSC 436 - Project 1
// Native C++ unit tests for the linear classifier.

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

// Include the actual classifier implementation.
// The original main() is excluded during unit testing.
#include "../src/linear_classifier.cpp"


// 1. Report failed tests.
void check(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error("Test failed: " + message);
    }
}


// 2. Compare floating-point values.
void check_close(double actual, double expected,
                 const std::string& message)
{
    constexpr double tolerance = 1e-9;

    check(
        std::isfinite(actual) &&
        std::fabs(actual - expected) < tolerance,
        message
    );
}


// 3. Add a simulated vehicle to a dataset.
void add_car(Dataset& dataset, int label,
             double price, double weight)
{
    Car car{};

    car.is_big_car = label;
    car.normalized_price = price;
    car.normalized_weight = weight;

    dataset.cars.push_back(car);
}


int main()
{
    try
    {
        // TEST 1: Linear classification.
        Car car{};

        car.normalized_price = 0.25;
        car.normalized_weight = 0.50;

        // Weighted sum = 2(0.25) + 0.50 = 1.00

        check(
            predict_car_size(car, 0.95) == 1,
            "Classification above threshold"
        );

        check(
            predict_car_size(car, 1.05) == 0,
            "Classification below threshold"
        );


        // TEST 2: Exact threshold boundary.
        check(
            predict_car_size(car, 1.00) == 1,
            "Classification exactly at threshold"
        );


        // TEST 3: Confusion matrix.
        Dataset dataset;

        // Two true positives.
        add_car(dataset, 1, 0.50, 0.25);
        add_car(dataset, 1, 0.75, 0.00);

        // One false positive.
        add_car(dataset, 0, 0.625, 0.00);

        // One false negative.
        add_car(dataset, 1, 0.125, 0.50);

        // Three true negatives.
        add_car(dataset, 0, 0.125, 0.125);
        add_car(dataset, 0, 0.25, 0.25);
        add_car(dataset, 0, 0.00, 0.00);

        const ConfusionMatrix cm = evaluate(dataset, 1.00);

        check(cm.tp == 2, "True positives");
        check(cm.fp == 1, "False positives");
        check(cm.fn == 1, "False negatives");
        check(cm.tn == 3, "True negatives");


        // TEST 4: Classification metrics.
        const Metrics metrics = calculate_metrics(cm);

        check_close(
            metrics.accuracy,
            100.0 * 5.0 / 7.0,
            "Accuracy"
        );

        check_close(
            metrics.error,
            100.0 * 2.0 / 7.0,
            "Error rate"
        );

        check_close(
            metrics.precision,
            100.0 * 2.0 / 3.0,
            "Precision"
        );

        check_close(
            metrics.recall,
            100.0 * 2.0 / 3.0,
            "Recall"
        );

        check_close(
            metrics.specificity,
            75.0,
            "Specificity"
        );

        check_close(
            metrics.false_positive_rate,
            25.0,
            "False positive rate"
        );

        check_close(
            metrics.false_negative_rate,
            100.0 / 3.0,
            "False negative rate"
        );

        check_close(
            metrics.f1,
            100.0 * 4.0 / 6.0,
            "F1-score"
        );


        // TEST 5: Undefined metrics.
        check(
            std::isnan(percent(5.0, 0.0)),
            "Division by zero should return NaN"
        );


        std::cout
            << "All linear classifier unit tests passed.\n";

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}