# Archived reference only; not used by the C++ build or tests.
# Name: Kannan Venkateswaran
# Refactor Date: 10/11/2026
# CMSC 436 - Project 1: Car Classification
# File: reference/r/linear_classifier.R
#
# Run from the project root:
#   Rscript reference/r/linear_classifier.R
#
# Classifier documented in the original confusion-matrix calculations:
#   big car (1) if 2 * normalized_price + normalized_weight >= threshold
#   small car (0) otherwise
#
# NOTE: This differs from the x + y = threshold lines drawn in
# data_analysis.R. Both equations appear in the original report.
# We preserve the classifier equation here because it reproduces
# the reported confusion matrices exactly.

# ---- 1. Load and normalize the original data ----------------------------
normalize_min_max <- function(values) {
  minimum <- min(values)
  maximum <- max(values)

  if (maximum == minimum) {
    stop("Cannot normalize a column with identical values.")
  }

  (values - minimum) / (maximum - minimum)
}

load_car_data <- function(group_name) {
  file_path <- file.path("data", paste0("group", group_name, ".txt"))
  if (!file.exists(file_path)) {
    stop(paste("Missing dataset:", file_path,
               "\nRun this script from the project root."))
  }

  # The .txt files are comma-separated, with no header row.
  cars <- read.csv(
    file_path,
    header = FALSE,
    col.names = c("price_usd", "weight_lbs", "is_big_car")
  )

  if (anyNA(cars) || !all(vapply(cars, is.numeric, logical(1)))) {
    stop(paste("Missing or nonnumeric values in Group", group_name))
  }
  if (!all(cars$is_big_car %in% c(0, 1))) {
    stop(paste("Invalid class labels in Group", group_name))
  }

  cars$price_normalized <- normalize_min_max(cars$price_usd)
  cars$weight_normalized <- normalize_min_max(cars$weight_lbs)
  cars
}

# ---- 2. Apply the linear classifier -------------------------------------
predict_car_size <- function(cars, threshold) {
  weighted_sum <- 2 * cars$price_normalized + cars$weight_normalized
  as.integer(weighted_sum >= threshold)
}

# ---- 3. Create a confusion matrix and calculate metrics ----------------
calculate_results <- function(actual, predicted, group_name, threshold) {
  # Rows = actual classes; columns = predicted classes.
  matrix <- table(
    Actual = factor(actual, levels = c(0, 1)),
    Predicted = factor(predicted, levels = c(0, 1))
  )

  tn <- as.integer(matrix["0", "0"])
  fp <- as.integer(matrix["0", "1"])
  fn <- as.integer(matrix["1", "0"])
  tp <- as.integer(matrix["1", "1"])

  # Undefined ratios are returned as NA, instead of causing division by zero.
  safe_rate <- function(numerator, denominator) {
    if (denominator == 0) return(NA_real_)
    numerator / denominator
  }

  accuracy <- safe_rate(tp + tn, tp + tn + fp + fn)
  precision <- safe_rate(tp, tp + fp)
  recall <- safe_rate(tp, tp + fn)  # True-positive rate
  specificity <- safe_rate(tn, tn + fp)  # True-negative rate
  f1 <- safe_rate(2 * tp, 2 * tp + fp + fn)

  metrics <- data.frame(
    group = group_name,
    threshold = threshold,
    observations = length(actual),
    TP = tp,
    FP = fp,
    FN = fn,
    TN = tn,
    accuracy_pct = round(100 * accuracy, 2),
    error_pct = round(100 * (1 - accuracy), 2),
    precision_pct = round(100 * precision, 2),
    recall_pct = round(100 * recall, 2),
    specificity_pct = round(100 * specificity, 2),
    false_positive_rate_pct = round(100 * (1 - specificity), 2),
    false_negative_rate_pct = round(100 * (1 - recall), 2),
    f1_pct = round(100 * f1, 2)
  )

  # Long format makes the 2x2 matrix easy to inspect in a CSV file.
  matrix_csv <- as.data.frame(matrix)
  matrix_csv$group <- group_name
  matrix_csv <- matrix_csv[, c("group", "Actual", "Predicted", "Freq")]
  names(matrix_csv) <- c("group", "actual", "predicted", "count")

  list(matrix = matrix, metrics = metrics, matrix_csv = matrix_csv)
}

# ---- 4. Process Groups A, B, and C --------------------------------------
# These thresholds come from the original assignment report.
thresholds <- c(A = 0.95, B = 1.02, C = 0.98)
all_metrics <- list()
all_matrices <- list()

for (group_name in names(thresholds)) {
  threshold <- thresholds[[group_name]]
  cars <- load_car_data(group_name)

  predicted <- predict_car_size(cars, threshold)
  results <- calculate_results(
    actual = cars$is_big_car,
    predicted = predicted,
    group_name = group_name,
    threshold = threshold
  )

  cat(sprintf("\nGroup %s (threshold = %.2f)\n", group_name, threshold))
  print(results$matrix)
  cat(sprintf("Accuracy: %.2f%% | Precision: %.2f%% | Recall: %.2f%% | F1: %.2f%%\n",
              results$metrics$accuracy_pct,
              results$metrics$precision_pct,
              results$metrics$recall_pct,
              results$metrics$f1_pct))

  all_metrics[[group_name]] <- results$metrics
  all_matrices[[group_name]] <- results$matrix_csv
}

# ---- 5. Save compact, GitHub-friendly results ---------------------------
output_dir <- "results"
dir.create(output_dir, recursive = TRUE, showWarnings = FALSE)

metrics_file <- file.path(output_dir, "classification_metrics.csv")
matrices_file <- file.path(output_dir, "confusion_matrices.csv")
write.csv(do.call(rbind, all_metrics), metrics_file, row.names = FALSE)
write.csv(do.call(rbind, all_matrices), matrices_file, row.names = FALSE)

cat("\nSaved:", metrics_file, "\n")
cat("Saved:", matrices_file, "\n")
cat("Done.\n")
