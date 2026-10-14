import csv
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

from pathlib import Path


# 1. Expected results from the original project

EXPECTED = {
    "A": {"TP": 2000, "FP": 8, "FN": 0, "TN": 1992},
    "B": {"TP": 2000, "FP": 1284, "FN": 0, "TN": 716},
    "C": {"TP": 1979, "FP": 1767, "FN": 21, "TN": 233},
}

EXPECTED_ACCURACY = {
    "A": 99.80,
    "B": 67.90,
    "C": 55.30,
}


def read_csv(path):
    with path.open(newline="") as file:
        return list(csv.DictReader(file))


# 2. Verify the linear classifier

def test_classifier(work):
    metrics = read_csv(
        work / "results/classification_metrics.csv"
    )

    matrices = read_csv(
        work / "results/confusion_matrices.csv"
    )

    assert len(metrics) == 3
    assert len(matrices) == 12

    for row in metrics:
        group = row["group"]
        assert group in EXPECTED

        assert int(row["observations"]) == 4000

        for metric, expected in EXPECTED[group].items():
            assert int(row[metric]) == expected, (
                f"Group {group}: incorrect {metric}"
            )

        assert abs(
            float(row["accuracy_pct"]) - EXPECTED_ACCURACY[group]
        ) < 0.01

    actual_matrices = {
        (
            row["group"],
            int(row["actual"]),
            int(row["predicted"])
        ): int(row["count"])
        for row in matrices
    }

    for group, counts in EXPECTED.items():
        expected_matrix = {
            (group, 0, 0): counts["TN"],
            (group, 0, 1): counts["FP"],
            (group, 1, 0): counts["FN"],
            (group, 1, 1): counts["TP"],
        }

        for key, expected in expected_matrix.items():
            assert actual_matrices[key] == expected

    print("PASS: All classifier results match the original project.")


# 3. Verify the McCulloch-Pitts neuron

def test_neuron(work):
    rows = read_csv(
        work / "results/mcculloch_pitts_truth_table.csv"
    )

    assert len(rows) == 8

    combinations = set()

    for row in rows:
        x = int(row["X"])
        y = int(row["Y"])
        z = int(row["Z"])

        assert x in (0, 1)
        assert y in (0, 1)
        assert z in (0, 1)

        combinations.add((x, y, z))

        expected_sum = -3 * x + 3 * y + z
        expected_output = int(x == 0 or y == 1)

        assert int(row["weighted_sum"]) == expected_sum
        assert int(row["neuron_output"]) == expected_output

        assert (
            int(row["expected_not_X_or_Y"]) == expected_output
        )

        comparison = ">=" if expected_output == 1 else "<"

        assert row["inequality"] == (
            f"{expected_sum} {comparison} -1"
        )

    assert len(combinations) == 8

    ranges = read_csv(
        work / "results/mcculloch_pitts_threshold_range.csv"
    )

    assert len(ranges) == 1

    result = ranges[0]

    assert int(result["weight_X"]) == -3
    assert int(result["weight_Y"]) == 3
    assert int(result["weight_Z"]) == 1

    assert int(result["original_threshold"]) == -1
    assert int(result["lower_exclusive"]) == -2
    assert int(result["upper_inclusive"]) == 0

    print("PASS: Neuron truth table and threshold range verified.")


# 4. Verify normalization through the generated SVG files

def test_normalization(work):
    svg_namespace = "{http://www.w3.org/2000/svg}"

    for group in "ABC":
        svg_file = (
            work / "results/figures"
            / f"group{group}_normalized.svg"
        )

        root = ET.parse(svg_file).getroot()

        # Vehicle points use radius 3.2.
        # Legend symbols use a different radius.
        points = [
            element
            for element in root.iter(svg_namespace + "circle")
            if element.get("r") == "3.2"
        ]

        assert len(points) == 4000

        colors = {"#2563EB": 0, "#EA580C": 0}

        xs = []
        ys = []

        for point in points:
            x = float(point.get("cx"))
            y = float(point.get("cy"))

            # Plot boundaries from data_analysis.cpp.
            assert 125 <= x <= 1065
            assert 120 <= y <= 780

            xs.append(x)
            ys.append(y)

            color = point.get("fill")
            assert color in colors
            colors[color] += 1

        assert colors["#2563EB"] == 2000
        assert colors["#EA580C"] == 2000

        # Min-max normalization should reach both endpoints.
        assert abs(min(xs) - 125) < 0.01
        assert abs(max(xs) - 1065) < 0.01
        assert abs(min(ys) - 120) < 0.01
        assert abs(max(ys) - 780) < 0.01

    print("PASS: Normalization and all three SVG graphs verified.")


# 5. Prepare test inputs and execute the real C++ program

def main():
    mode = sys.argv[1]
    executable = Path(sys.argv[2]).resolve()
    original_data = Path(sys.argv[3]).resolve()

    negative_cases = {
        "missing": "Cannot open",
        "empty": "Dataset is empty",
        "malformed": "data/groupA.txt:1:",
        "constant": "Unable to normalize",
        "invalid_label": "Class label must be 0 or 1",
    }

    with tempfile.TemporaryDirectory() as directory:
        work = Path(directory)

        if mode in ("classifier", "plots"):
            shutil.copytree(original_data, work / "data")

        elif mode in negative_cases:
            data_dir = work / "data"
            data_dir.mkdir()

            fixtures = {
                "empty": "",
                "malformed": "10000,not_a_number,0\n",
                "constant": "100,200,0\n100,400,1\n",
                "invalid_label": "100,200,2\n",
            }

            if mode != "missing":
                (data_dir / "groupA.txt").write_text(
                    fixtures[mode]
                )

        result = subprocess.run(
            [str(executable)],
            cwd=work,
            capture_output=True,
            text=True,
        )

        if mode in negative_cases:
            assert result.returncode != 0, (
                "Program should reject invalid input."
            )

            assert negative_cases[mode] in result.stderr, (
                f"Unexpected error message: {result.stderr}"
            )

            print(f"PASS: {mode} input rejected correctly.")
            return

        assert result.returncode == 0, result.stderr

        if mode == "classifier":
            test_classifier(work)

        elif mode == "neuron":
            test_neuron(work)

        elif mode == "plots":
            test_normalization(work)

        else:
            raise ValueError(f"Unknown test mode: {mode}")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"FAIL: {error}", file=sys.stderr)
        sys.exit(1)