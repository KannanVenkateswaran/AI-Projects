# Name: Kannan Venkateswaran
# Refactor Date: 10/11/2026
# CMSC 436 - Project 1: Car Classification
# File: src/mcculloch_pitts.R
#
# Run from the project root:
#   Rscript src/mcculloch_pitts.R
#
# The original neuron has three binary inputs (X, Y, Z), weights
# (-3, +3, +1), and threshold -1. It uses a unipolar hard-threshold
# activation function:
#   weighted_sum = -3*X + 3*Y + Z
#   output = 1 if weighted_sum >= threshold; otherwise output = 0
#
# The resulting Boolean function is NOT X OR Y (Z does not affect
# the final output for this range of thresholds).

# ---- 1. Define the original neuron -------------------------------------
weights <- c(X = -3, Y = 3, Z = 1)
threshold <- -1

# This function accepts vectors, so it works on all truth-table rows.
hard_threshold <- function(weighted_sum, threshold) {
  as.integer(weighted_sum >= threshold)
}

# ---- 2. Generate every possible binary input combination --------------
# 2^3 = 8 rows. Sorting gives 000, 001, 010, ..., 111.
inputs <- expand.grid(X = 0:1, Y = 0:1, Z = 0:1)
inputs <- inputs[order(inputs$X, inputs$Y, inputs$Z), ]
rownames(inputs) <- NULL

# Compute the weighted sum and the predicted neuron output for every row.
weighted_sum <- (
  weights[["X"]] * inputs$X +
  weights[["Y"]] * inputs$Y +
  weights[["Z"]] * inputs$Z
)
output <- hard_threshold(weighted_sum, threshold)

# ---- 3. Verify the equivalent Boolean expression ----------------------
# NOT X OR Y: output 0 only when X = 1 and Y = 0.
# The expression does not depend on Z.
expected_output <- as.integer(inputs$X == 0 | inputs$Y == 1)

# A comparison expression makes each threshold decision visible.
inequality <- sprintf(
  "%d %s %d",
  as.integer(weighted_sum),
  ifelse(output == 1, ">=", "<"),
  threshold
)

truth_table <- data.frame(
  X = inputs$X,
  Y = inputs$Y,
  Z = inputs$Z,
  weighted_sum = weighted_sum,
  inequality = inequality,
  neuron_output = output,
  expected_not_X_or_Y = expected_output
)

# Stop immediately if the neuron does not match NOT X OR Y for any row.
stopifnot(all(truth_table$neuron_output == truth_table$expected_not_X_or_Y))

cat("McCulloch-Pitts neuron: -3*X + 3*Y + Z >= -1\n")
cat("Boolean function: NOT X OR Y\n\n")
print(truth_table, row.names = FALSE)
cat("\nVerification passed: all 8 rows match NOT X OR Y.\n")

# ---- 4. Derive the complete valid threshold range ---------------------
# For every expected output 0, the weighted sum must be LESS than T.
# For every expected output 1, the weighted sum must be >= T.
# Therefore: max(score for output 0) < T <= min(score for output 1).
zero_scores <- weighted_sum[expected_output == 0]
one_scores <- weighted_sum[expected_output == 1]

lower_exclusive <- max(zero_scores)
upper_inclusive <- min(one_scores)

if (lower_exclusive >= upper_inclusive) {
  stop("No threshold can implement the expected Boolean function.")
}

# Confirm that the original threshold belongs to the calculated interval.
stopifnot(threshold > lower_exclusive, threshold <= upper_inclusive)

# Check the end points: the lower endpoint is NOT valid; upper IS valid.
stopifnot(
  !identical(hard_threshold(weighted_sum, lower_exclusive), expected_output),
  identical(hard_threshold(weighted_sum, upper_inclusive), expected_output)
)

cat(sprintf(
  "\nValid threshold range: %g < T <= %g\n",
  lower_exclusive, upper_inclusive
))
cat(sprintf("Original threshold: T = %g (valid)\n", threshold))

# ---- 5. Export reproducible results ------------------------------------
output_dir <- "results"
dir.create(output_dir, recursive = TRUE, showWarnings = FALSE)

truth_table_file <- file.path(output_dir, "mcculloch_pitts_truth_table.csv")
threshold_file <- file.path(output_dir, "mcculloch_pitts_threshold_range.csv")

threshold_results <- data.frame(
  weight_X = weights[["X"]],
  weight_Y = weights[["Y"]],
  weight_Z = weights[["Z"]],
  original_threshold = threshold,
  lower_exclusive = lower_exclusive,
  upper_inclusive = upper_inclusive,
  logic_function = "NOT X OR Y"
)

write.csv(truth_table, truth_table_file, row.names = FALSE)
write.csv(threshold_results, threshold_file, row.names = FALSE)

cat("\nSaved:", truth_table_file, "\n")
cat("Saved:", threshold_file, "\n")
cat("Done.\n")
