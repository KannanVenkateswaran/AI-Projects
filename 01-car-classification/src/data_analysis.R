# Name: Kannan Venkateswaran
# Refactor Date: 10/11/2026
# CMSC 436 - Project 1: Car Classification
# File: Data Analysis (data_analysis.R)
#
# Run from the 01-car-classification project folder:
#   Rscript src/data_analysis.R
#
# This script:
#   1. Loads Groups A, B, and C.
#   2. Normalizes price and weight separately to the range [0, 1].
#   3. Plots small cars (0) and big cars (1), along with the
#      manually chosen separation lines documented in the report.
#
# Classification predictions and confusion matrices belong in the
# next file, linear_classifier.R.

# ---- 1. Define a min-max normalization function -------------------------
normalize_min_max <- function(values) {
  minimum <- min(values)
  maximum <- max(values)

  if (maximum == minimum) {
    stop("Cannot normalize a column where all values are identical.")
  }

  (values - minimum) / (maximum - minimum)
}

# ---- 2. Load one dataset ------------------------------------------------
load_car_data <- function(group_name) {
  file_path <- file.path("data", paste0("group", group_name, ".txt"))

  if (!file.exists(file_path)) {
    stop(
      paste0(
        "Dataset not found: ", file_path, "\n",
        "Run this script from the 01-car-classification folder.\n",
        "Current working directory: ", getwd()
      )
    )
  }

  # The original .txt files are comma-separated and have NO header row.
  cars <- read.csv(
    file_path,
    header = FALSE,
    col.names = c("price_usd", "weight_lbs", "is_big_car")
  )

  if (anyNA(cars)) {
    stop(paste("Missing or invalid values found in Group", group_name))
  }

  if (!all(cars$is_big_car %in% c(0, 1))) {
    stop(paste("Unexpected class labels found in Group", group_name))
  }

  # Normalize each numeric feature independently, just as in the report.
  cars$price_normalized <- normalize_min_max(cars$price_usd)
  cars$weight_normalized <- normalize_min_max(cars$weight_lbs)

  cars
}

# ---- 3. Generate a visualization ----------------------------------------
plot_car_data <- function(cars, group_name, threshold) {
  output_dir <- file.path("results", "figures")
  dir.create(output_dir, recursive = TRUE, showWarnings = FALSE)

  output_file <- file.path(
    output_dir,
    paste0("group", group_name, "_normalized.png")
  )

  # Save the plot as an image suitable for the GitHub README.
  png(filename = output_file, width = 1200, height = 900, res = 150)

  small_color <- "#2563EB"
  big_color <- "#EA580C"
  point_colors <- ifelse(cars$is_big_car == 0, small_color, big_color)

  plot(
    cars$price_normalized,
    cars$weight_normalized,
    col = adjustcolor(point_colors, alpha.f = 0.55),
    pch = 16,
    cex = 0.65,
    xlim = c(0, 1),
    ylim = c(0, 1),
    xlab = "Normalized price",
    ylab = "Normalized weight",
    main = paste("Group", group_name, "- Normalized car data")
  )

  # These are the hand-chosen separation lines from the original report:
  #   Group A: y = -x + 0.95
  #   Group B: y = -x + 1.02
  #   Group C: y = -x + 0.98
  # They are visualization lines, not the classification code.
  abline(a = threshold, b = -1, col = "#111827", lwd = 2)

  legend(
    "topleft",
    legend = c("Small car (0)", "Big car (1)", "Original separation line"),
    col = c(small_color, big_color, "#111827"),
    pch = c(16, 16, NA),
    lty = c(0, 0, 1),
    lwd = c(1, 1, 2),
    bty = "n",
    cex = 0.85
  )

  dev.off()
  cat("Saved plot:", output_file, "\n")
}

# ---- 4. Process all three groups ----------------------------------------
# Thresholds taken from the separation lines in the original project report.
thresholds <- c(A = 0.95, B = 1.02, C = 0.98)

for (group_name in names(thresholds)) {
  cars <- load_car_data(group_name)

  cat("\nGroup", group_name, "\n")
  cat("  Observations:", nrow(cars), "\n")
  cat("  Small cars:", sum(cars$is_big_car == 0), "\n")
  cat("  Big cars:", sum(cars$is_big_car == 1), "\n")
  cat("  Price range (USD):", min(cars$price_usd), "to", max(cars$price_usd), "\n")
  cat("  Weight range (lbs):", min(cars$weight_lbs), "to", max(cars$weight_lbs), "\n")

  plot_car_data(cars, group_name, thresholds[[group_name]])
}

cat("\nFinished! Open results/figures/ to view the graphs.\n")