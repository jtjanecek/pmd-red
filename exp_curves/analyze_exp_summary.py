#!/usr/bin/env python3
"""
Analyze and summarize Pokemon EXP curve data.

This script reads the pokemon_exp_curves.csv file and provides analysis
of growth rates, patterns, and statistics.
"""

import csv
from collections import defaultdict
from pathlib import Path


def load_exp_curves(csv_path):
    """Load EXP curve data from CSV."""
    curves = defaultdict(list)

    with open(csv_path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            species_id = int(row['species_id'])
            curves[species_id].append({
                'species_name': row['species_name'],
                'level': int(row['level']),
                'exp_required': int(row['exp_required']),
                'gain_hp': int(row['gain_hp']),
                'gain_att': int(row['gain_att']),
                'gain_sp_att': int(row['gain_sp_att']),
                'gain_def': int(row['gain_def']),
                'gain_sp_def': int(row['gain_sp_def'])
            })

    return curves


def classify_growth_rate(exp_100):
    """Classify growth rate based on EXP to level 100."""
    if exp_100 < 800000:
        return "Erratic"
    elif exp_100 < 900000:
        return "Fast"
    elif exp_100 < 1000000:
        return "Medium Fast"
    elif exp_100 < 1050000:
        return "Medium Slow"
    elif exp_100 < 1200000:
        return "Slow"
    else:
        return "Fluctuating"


def main():
    csv_path = Path('pokemon_exp_curves.csv')
    if not csv_path.exists():
        print(f"Error: {csv_path} not found. Run analyze_exp_curves.py first.")
        return

    print("Loading EXP curve data...")
    curves = load_exp_curves(csv_path)

    # Calculate statistics for each Pokemon
    species_stats = []

    for species_id, levels in curves.items():
        if not levels:
            continue

        species_name = levels[0]['species_name']

        # Find exp to level 100
        exp_100 = next((l['exp_required'] for l in levels if l['level'] == 100), None)
        if exp_100 is None:
            continue

        # Calculate total stat gains
        total_hp_gain = sum(l['gain_hp'] for l in levels)
        total_att_gain = sum(l['gain_att'] for l in levels)
        total_sp_att_gain = sum(l['gain_sp_att'] for l in levels)
        total_def_gain = sum(l['gain_def'] for l in levels)
        total_sp_def_gain = sum(l['gain_sp_def'] for l in levels)

        species_stats.append({
            'species_id': species_id,
            'species_name': species_name,
            'exp_to_100': exp_100,
            'growth_rate': classify_growth_rate(exp_100),
            'total_hp': total_hp_gain,
            'total_att': total_att_gain,
            'total_sp_att': total_sp_att_gain,
            'total_def': total_def_gain,
            'total_sp_def': total_sp_def_gain
        })

    # Sort by exp to 100
    species_stats.sort(key=lambda x: x['exp_to_100'])

    # Print summary statistics
    print("\n" + "="*80)
    print("EXP CURVE ANALYSIS SUMMARY")
    print("="*80)

    # Growth rate distribution
    growth_rate_counts = defaultdict(int)
    for s in species_stats:
        growth_rate_counts[s['growth_rate']] += 1

    print("\nGrowth Rate Distribution:")
    print("-" * 40)
    for rate, count in sorted(growth_rate_counts.items(), key=lambda x: -x[1]):
        print(f"  {rate:20s}: {count:3d} Pokemon")

    # Fastest growing Pokemon
    print("\nTop 10 Fastest Growing Pokemon (least EXP to 100):")
    print("-" * 80)
    print(f"{'ID':>4} {'Name':25} {'EXP to 100':>12} {'Growth Rate':15}")
    print("-" * 80)
    for s in species_stats[:10]:
        print(f"{s['species_id']:4d} {s['species_name']:25} {s['exp_to_100']:12,} {s['growth_rate']:15}")

    # Slowest growing Pokemon
    print("\nTop 10 Slowest Growing Pokemon (most EXP to 100):")
    print("-" * 80)
    print(f"{'ID':>4} {'Name':25} {'EXP to 100':>12} {'Growth Rate':15}")
    print("-" * 80)
    for s in species_stats[-10:]:
        print(f"{s['species_id']:4d} {s['species_name']:25} {s['exp_to_100']:12,} {s['growth_rate']:15}")

    # Unique EXP curves
    exp_curves = defaultdict(list)
    for s in species_stats:
        exp_curves[s['exp_to_100']].append(s['species_name'])

    print(f"\nUnique EXP Curves: {len(exp_curves)}")
    print(f"Total Pokemon Analyzed: {len(species_stats)}")

    # Show some interesting Pokemon
    print("\nSelected Pokemon EXP Requirements:")
    print("-" * 80)
    print(f"{'Name':25} {'EXP to 100':>12} {'Growth Rate':15}")
    print("-" * 80)

    interesting_pokemon = ['Bulbasaur', 'Charmander', 'Squirtle', 'Pikachu',
                          'Eevee', 'Mewtwo', 'Mew', 'Chikorita', 'Cyndaquil',
                          'Totodile', 'Treecko', 'Torchic', 'Mudkip']

    for name in interesting_pokemon:
        matches = [s for s in species_stats if s['species_name'] == name]
        if matches:
            s = matches[0]
            print(f"{s['species_name']:25} {s['exp_to_100']:12,} {s['growth_rate']:15}")

    # Export summary CSV
    summary_path = Path('pokemon_exp_summary.csv')
    with open(summary_path, 'w', newline='') as f:
        fieldnames = ['species_id', 'species_name', 'exp_to_100', 'growth_rate',
                     'total_hp', 'total_att', 'total_sp_att', 'total_def', 'total_sp_def']
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(species_stats)

    print(f"\nSummary exported to: {summary_path}")

    # Show level progression for a few Pokemon
    print("\n" + "="*80)
    print("EXAMPLE: Bulbasaur Level Progression")
    print("="*80)
    print(f"{'Level':>5} {'EXP Required':>12} {'HP Gain':>8} {'Att':>5} {'SpAtt':>6} {'Def':>5} {'SpDef':>6}")
    print("-" * 80)

    bulbasaur = next((c for c in curves.values() if c[0]['species_name'] == 'Bulbasaur'), None)
    if bulbasaur:
        for level in [1, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]:
            entry = next((l for l in bulbasaur if l['level'] == level), None)
            if entry:
                print(f"{entry['level']:5d} {entry['exp_required']:12,} "
                      f"{entry['gain_hp']:8d} {entry['gain_att']:5d} {entry['gain_sp_att']:6d} "
                      f"{entry['gain_def']:5d} {entry['gain_sp_def']:6d}")

    print("\n" + "="*80)


if __name__ == '__main__':
    main()
