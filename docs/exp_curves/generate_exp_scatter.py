#!/usr/bin/env python3
"""
Generate a 1D scatter plot of total EXP required at a given level for every Pokemon.

Edit LEVEL_FILTER below to choose which level to visualize. The script reads
pokemon_exp_curves.csv in this directory and writes a 4K image with one labeled
point per species. To reduce label/dot overlap, each species gets a small,
deterministic vertical jitter and duplicate names receive numbered suffixes.
"""

import csv
import hashlib
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt

# Level to plot; change this to visualize another level.
LEVEL_FILTER = 20

# Paths and output settings.
CSV_PATH = Path(__file__).with_name("pokemon_exp_curves.csv")
OUTPUT_PATH = Path(__file__).with_name(f"exp_scatter_level{LEVEL_FILTER}.png")
FIGURE_RESOLUTION = (3840, 2160)  # pixels (width, height)
DPI = 200  # used with FIGURE_RESOLUTION to size the figure

# Vertical jitter keeps the plot essentially 1D while avoiding overlap.
JITTER_SCALE = 0.6
LABEL_OFFSET = 0.04


def load_level_data(level):
    """Return a list of dicts with name/exp for the requested level."""
    rows = []

    with CSV_PATH.open(newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            if int(row["level"]) == level:
                rows.append(
                    {
                        "name": row["species_name"],
                        "exp": int(row["exp_required"]),
                    }
                )

    if not rows:
        raise ValueError(f"No data found for level {level}. Check LEVEL_FILTER.")

    rows.sort(key=lambda entry: entry["exp"])
    return rows


def assign_labels(entries):
    """Add a label field, numbering duplicates so forms are distinct."""
    name_counts = defaultdict(int)
    for entry in entries:
        name_counts[entry["name"]] += 1

    seen = defaultdict(int)
    for entry in entries:
        name = entry["name"]
        seen[name] += 1
        count = name_counts[name]
        if count > 1:
            entry["label"] = f"{name} ({seen[name]})"
        else:
            entry["label"] = name


def deterministic_jitter(label, scale):
    """Return a small, repeatable jitter based on the label text."""
    digest = hashlib.sha256(label.encode("utf-8")).digest()
    value = int.from_bytes(digest[:8], "big") / (2**64)
    return (value * 2 - 1) * scale


def main():
    data = load_level_data(LEVEL_FILTER)
    assign_labels(data)

    exp_values = [entry["exp"] for entry in data]
    labels = [entry["label"] for entry in data]
    y_positions = [deterministic_jitter(label, JITTER_SCALE) for label in labels]

    width_inches = FIGURE_RESOLUTION[0] / DPI
    height_inches = FIGURE_RESOLUTION[1] / DPI

    fig, ax = plt.subplots(figsize=(width_inches, height_inches), dpi=DPI)

    # Scatter points with slight jitter to keep a 1D look while separating collisions.
    ax.scatter(exp_values, y_positions, color="#2563eb", s=18, alpha=0.85, zorder=2)
    ax.axhline(0, color="#9ca3af", linewidth=1, zorder=1)

    # Label each point with the Pokemon name (or numbered form).
    for x, y, label in zip(exp_values, y_positions, labels):
        va = "bottom" if y >= 0 else "top"
        ax.text(
            x,
            y + (LABEL_OFFSET if y >= 0 else -LABEL_OFFSET),
            label,
            fontsize=7,
            ha="right",
            va=va,
            rotation=45,
            rotation_mode="anchor",
            color="#111827",
        )

    # Keep the plot visually 1D.
    spread = max(JITTER_SCALE + LABEL_OFFSET * 6, 0.8)
    ax.set_ylim(-spread, spread)
    ax.set_yticks([])
    ax.set_ylabel("Vertical jitter (for legibility)")

    margin = max(500, (max(exp_values) - min(exp_values)) * 0.02)
    ax.set_xlim(min(exp_values) - margin, max(exp_values) + margin)
    ax.set_xlabel(f"Total EXP required to reach level {LEVEL_FILTER}")
    ax.set_title(f"EXP needed per species at level {LEVEL_FILTER} (PMD Red)")

    for spine in ["right", "top", "left"]:
        ax.spines[spine].set_visible(False)

    fig.tight_layout()
    fig.savefig(OUTPUT_PATH, dpi=DPI)
    print(f"Saved plot to {OUTPUT_PATH} at {FIGURE_RESOLUTION[0]}x{FIGURE_RESOLUTION[1]} resolution.")


if __name__ == "__main__":
    main()
