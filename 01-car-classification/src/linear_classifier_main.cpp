// CMSC 436 - Project 1
// Linear classifier application entry point.

#include "linear_classifier.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace fs = std::filesystem;

int main()
{
    try
    {
        const std::array<std::pair<char, double>, 3> groups{
            {{'A', 0.95}, {'B', 1.02}, {'C', 0.98}}};

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
        metrics_file << std::fixed << std::setprecision(2);
        std::cout << std::fixed << std::setprecision(2);

        for (const auto& [name, threshold] : groups)
        {
            const fs::path input_file =
                fs::path("data") / ("group" + std::string(1, name) + ".txt");
            const Dataset dataset = load_car_data(input_file);
            const ConfusionMatrix cm = evaluate(dataset, threshold);
            const Metrics m = calculate_metrics(cm);

            std::cout << "\nGroup " << name << " (threshold = " << threshold << ")\n"
                      << "                  Predicted 0   Predicted 1\n"
                      << "  Actual 0       " << cm.tn << "          " << cm.fp << '\n'
                      << "  Actual 1       " << cm.fn << "          " << cm.tp << '\n'
                      << "  Accuracy: " << m.accuracy << "% | Precision: " << m.precision
                      << "% | Recall: " << m.recall << "% | F1: " << m.f1 << "%\n";

            metrics_file << name << ',' << threshold << ',' << dataset.cars.size() << ',' << cm.tp
                         << ',' << cm.fp << ',' << cm.fn << ',' << cm.tn << ',' << m.accuracy << ','
                         << m.error << ',' << m.precision << ',' << m.recall << ',' << m.specificity
                         << ',' << m.false_positive_rate << ',' << m.false_negative_rate << ','
                         << m.f1 << '\n';

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
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
