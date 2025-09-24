
from raw_effects import EFFECTS

import pandas as pd

df = pd.read_csv('pmd_moves.csv', dtype=str)

# Normalize df names -> uppercase, spaces to underscores, prefix with MOVE_
df_moves = {"MOVE_" + name.strip().upper().replace(" ", "_").replace("-", "_") 
            for name in df["name"]}

# Extract move keys from EFFECTS
effect_moves = {list(d.keys())[0] for d in EFFECTS}

# Compare
missing_in_effects = df_moves - effect_moves
missing_in_df = effect_moves - df_moves

print("Moves in CSV but not in EFFECTS:")
print(sorted(missing_in_effects))

print("\nMoves in EFFECTS but not in CSV:")
print(sorted(missing_in_df))

# Keep only EFFECTS that are present in df_moves
filtered_effects = [
    effect for effect in EFFECTS if list(effect.keys())[0] in df_moves
]

from raw_effects import EFFECTS
import pandas as pd
import numpy as np
import re

# You already have df, df_moves, effect_moves, filtered_effects defined above

# --- Normalize names in df to keys ---
def to_move_key(s: str) -> str:
    if pd.isna(s):
        return None
    s = re.sub(r"\s+", " ", s.strip())
    return "MOVE_" + s.upper().replace("-", "_").replace(" ", "_")

df["move_key"] = df["name"].apply(to_move_key)

# --- Convert FILTERED effects -> DataFrame (only moves present in df) ---
rows = []
for d in filtered_effects:  # <-- use filtered_effects
    move_key, payload = list(d.items())[0]
    row = {"move_key": move_key}
    row.update(payload)
    rows.append(row)

effects_df = pd.DataFrame(rows)

# Coerce booleans to 0/1 and numerics where possible for clean comparisons
for col in effects_df.columns:
    if col == "move_key":
        continue
    if effects_df[col].dtype == bool:
        effects_df[col] = effects_df[col].astype(int)
    else:
        effects_df[col] = pd.to_numeric(effects_df[col], errors="ignore")

# --- Join on move_key ---
merged = df.merge(effects_df, on="move_key", how="inner", suffixes=("_csv", "_eff"))

# --- Parse flags -> one-hot columns ---
def split_flags(s):
    if pd.isna(s) or not str(s).strip():
        return []
    return [p for p in re.split(r"[|;,/\s]+", str(s).strip()) if p]

merged["_flags_list"] = merged.get("other_flags", pd.Series(index=merged.index, dtype=object)).apply(split_flags)
all_flags = sorted({f for flist in merged["_flags_list"] for f in flist})

for flg in all_flags:
    merged[f"flag__{flg}"] = merged["_flags_list"].apply(lambda xs: int(flg in xs))

# --- Helper: contingency summary for 1:1 checks ---
def contingency(a, b):
    t = pd.crosstab(a, b, dropna=False)
    return t.reindex(index=sorted(a.unique()), columns=sorted(b.unique()), fill_value=0)

# --- 1:1 mapping checks (effect field vs each flag) ---
effect_cols = [c for c in effects_df.columns if c != "move_key"]
flag_cols = [c for c in merged.columns if c.startswith("flag__")]

rows_summary = []
for ecol in effect_cols:
    # Try to coerce to numeric for clean equality vs 0/1 flags
    x = pd.to_numeric(merged[ecol], errors="ignore")

    for fcol in flag_cols:
        y = merged[fcol].astype(int)

        # Only consider binary effect fields (0/1 or True/False) for direct 1:1 with flags
        # If x has only {0,1} (or NaN), treat it as binary; otherwise skip equality test
        x_binable = pd.api.types.is_numeric_dtype(x) and set(pd.Series(x).dropna().unique()).issubset({0,1})

        # Exact equality (only meaningful if x is binary)
        exact_eq = bool((x.fillna(-9999) == y.fillna(-9999)).all()) if x_binable else False

        # Unique combinations (good to inspect even when not binary)
        combos = merged[[ecol, fcol]].dropna()
        uniq = combos.drop_duplicates().values.tolist()

        # Contingency (useful to see mismatches)
        ct = contingency(pd.Series(x).fillna(-9999), y)

        rows_summary.append({
            "effect_field": ecol,
            "flag": fcol.replace("flag__", ""),
            "n": len(combos),
            "effect_values": sorted(pd.Series(x).dropna().unique().tolist()),
            "flag_values": sorted(y.dropna().unique().tolist()),
            "binary_effect": x_binable,
            "exact_match": exact_eq,
            "unique_combinations": uniq,
            "contingency": ct
        })

summary = pd.DataFrame(rows_summary)

# Heuristic filter for promising 1:1s:
#  - either exact_match = True
#  - or only two unique combos and they are aligned like [ [0,0], [1,1] ]
def looks_like_identity(uniq):
    # Accept [ [0,0] ] or [ [1,1] ] (degenerate) or [ [0,0],[1,1] ]
    pairs = {(int(a), int(b)) for a,b in uniq if pd.notna(a) and pd.notna(b)}
    return pairs.issubset({(0,0),(1,1)}) and len(pairs) in {1,2}

candidates = summary[
    (summary["binary_effect"]) &
    (
        (summary["exact_match"]) |
        (summary["unique_combinations"].apply(looks_like_identity))
    )
].sort_values(["exact_match","n"], ascending=[False, False])

print("\n=== Candidate 1:1 mappings (filtered_effects only) ===")
with pd.option_context("display.max_colwidth", 120):
    print(
        candidates[["effect_field","flag","n","exact_match","effect_values","unique_combinations"]]
        .head(50)
        .to_string(index=False)
    )

# Optional: inspect specific pairs with contingency tables
# Example:
# pair = candidates.iloc[0]
# print(pair["effect_field"], pair["flag"])
# print(summary[(summary.effect_field==pair["effect_field"]) & (summary.flag==pair["flag"])].iloc[0]["contingency"])

# (Optional) Save a flat view to CSV for manual review
flat = summary.drop(columns=["contingency"]).copy()
flat.to_csv("effect_flag_1to1_checks.csv", index=False)

print("\nCounts:", {
    "EFFECTS_total": len(EFFECTS),
    "EFFECTS_filtered": len(filtered_effects),
    "df_total": len(df),
    "merged_rows": len(merged),
    "num_flags": len(all_flags),
})
# Only compare unk* columns
effect_cols = [c for c in effects_df.columns if c.startswith("unk")]
flag_cols = [c for c in merged.columns if c.startswith("flag__")]

rows_summary = []
for ecol in effect_cols:
    x = merged[ecol].astype(str)
    for fcol in flag_cols:
        y = merged[fcol].astype(str)

        # Drop NaNs
        sub = merged[[ecol, fcol]].dropna()
        if sub.empty:
            continue

        # Percent agreement
        agreement = (sub[ecol] == sub[fcol]).mean()

        rows_summary.append({
            "effect_field": ecol,
            "flag": fcol.replace("flag__", ""),
            "n": len(sub),
            "agreement": agreement,
            "unique_combinations": sub.drop_duplicates().values.tolist()
        })

summary = pd.DataFrame(rows_summary)

# Filter to ≥90% agreement
candidates = summary[summary["agreement"] >= 0.9].sort_values("agreement", ascending=False)

print("\n=== Potential mappings (≥90% agreement) ===")
print(candidates.to_string(index=False, max_colwidth=80))
