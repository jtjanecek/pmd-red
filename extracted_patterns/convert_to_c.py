#!/usr/bin/env python3
"""
Convert raw fixed room logs to clean C format with ASCII visualization.
"""

import re
import os
from pathlib import Path

# Tile type to character mapping for visualization
TILE_CHARS = {
    0: ' ',   # Unused/uninitialized
    2: '#',   # Wall
    4: 'S',   # Spawn point (stairs down)
    6: '+',   # Secondary wall
    10: '~',  # Water
    16: 'P',  # Player spawn
    17: '!',  # Special tile
    22: '1',  # Stairs component (top-left)
    23: '2',  # Stairs component (top-center)
    24: '3',  # Stairs component (top-right)
    25: '4',  # Stairs component (center)
    26: '5',  # Stairs component (bottom-left)
    27: '6',  # Stairs component (bottom-right)
    60: '.',  # Open floor
    68: 'T',  # Trap/Item
}

def parse_room_log(log_path):
    """Parse a room log file and extract tile data."""
    with open(log_path, 'r') as f:
        content = f.read()

    # Extract room info
    start_match = re.search(r'\[FixedRoom\] PATTERN EXTRACTION START \(room (\d+), size (\d+)x(\d+)\)', content)
    if not start_match:
        return None

    room_num = int(start_match.group(1))
    # The log format is "size HxW" where H=height (rows), W=width (columns)
    # But Tile[Y][X] uses Y for rows and X for columns
    # So first number is number of rows (height), second is number of columns (width)
    height = int(start_match.group(2))  # rows
    width = int(start_match.group(3))   # columns

    # Extract all tile data
    tiles = [[0 for _ in range(width)] for _ in range(height)]
    tile_matches = re.finditer(r'Tile\[(\d+)\]\[(\d+)\] = (\d+)', content)

    for match in tile_matches:
        y = int(match.group(1))
        x = int(match.group(2))
        value = int(match.group(3))
        if y < height and x < width:
            tiles[y][x] = value

    return {
        'room_num': room_num,
        'width': width,
        'height': height,
        'tiles': tiles
    }

def render_ascii_map(tiles, width, height, show_numbers=False):
    """Render the tile map as ASCII art."""
    if show_numbers:
        # Show numeric values in a grid
        lines = []
        for y in range(height):
            line = []
            for x in range(width):
                line.append(f"{tiles[y][x]:3d}")
            lines.append(' '.join(line))
        return lines
    else:
        # Show character representation
        lines = []
        for y in range(height):
            line = []
            for x in range(width):
                tile = tiles[y][x]
                char = TILE_CHARS.get(tile, '?')
                line.append(char)
            lines.append(''.join(line))
        return lines

def generate_c_array(room_data):
    """Generate C array format for the room."""
    tiles = room_data['tiles']
    height = room_data['height']
    width = room_data['width']
    room_num = room_data['room_num']

    output = []
    output.append(f"// Room {room_num} - {width}x{height}")
    output.append("// ASCII Map:")

    # Add ASCII visualization
    ascii_map = render_ascii_map(tiles, width, height, show_numbers=False)
    for line in ascii_map:
        output.append(f"// {line}")

    output.append("")
    output.append("// Numeric Map (for unknown tiles marked with ?):")
    numeric_map = render_ascii_map(tiles, width, height, show_numbers=True)
    for line in numeric_map:
        output.append(f"// {line}")

    output.append("")
    output.append("// Tile Legend:")
    output.append("//   . = Open floor (60)")
    output.append("//   # = Wall (2)")
    output.append("//   + = Secondary wall (6)")
    output.append("//   ~ = Water (10)")
    output.append("//   P = Player spawn (16)")
    output.append("//   S = Stairs down (4)")
    output.append("//   ! = Special tile (17)")
    output.append("//   T = Trap/Item (68)")
    output.append("//   1-6 = Stairs components (22-27)")
    output.append("//   ? = Unknown (see numeric map above)")
    output.append("")

    # Generate C array
    output.append(f"u8 fixed_room_{room_num}_tiles[{height}][{width}] = {{")

    for y in range(height):
        row_values = []
        for x in range(width):
            row_values.append(f"{tiles[y][x]:3d}")

        if y == height - 1:
            output.append(f"    {{{', '.join(row_values)}}}")
        else:
            output.append(f"    {{{', '.join(row_values)}}},")

    output.append("};")
    output.append("")

    return '\n'.join(output)

def main():
    script_dir = Path(__file__).parent
    raw_dir = script_dir / 'raw'

    if not raw_dir.exists():
        print(f"Error: {raw_dir} does not exist")
        return

    # Get all room log files
    log_files = sorted(raw_dir.glob('room*.log'), key=lambda x: int(re.search(r'room(\d+)', x.name).group(1)))

    if not log_files:
        print(f"No room log files found in {raw_dir}")
        return

    print(f"Found {len(log_files)} room log files")

    # Process each room
    all_rooms = []
    for log_file in log_files:
        room_data = parse_room_log(log_file)
        if room_data:
            all_rooms.append(room_data)
            print(f"  Parsed room {room_data['room_num']}: {room_data['width']}x{room_data['height']}")

    # Generate output file
    output_file = script_dir / 'fixed_rooms.c'
    with open(output_file, 'w') as f:
        f.write("// Fixed Room Patterns\n")
        f.write("// Auto-generated from extracted logs\n")
        f.write("//\n")
        f.write("// This file contains the tile layouts for fixed rooms.\n")
        f.write("// Each room includes an ASCII visualization for easy viewing.\n")
        f.write("//\n\n")

        for room_data in all_rooms:
            f.write(generate_c_array(room_data))
            f.write("\n\n")

    print(f"\nGenerated {output_file}")
    print(f"Total rooms processed: {len(all_rooms)}")

    # Also generate a summary file with just the ASCII maps
    summary_file = script_dir / 'room_visualizations.txt'
    with open(summary_file, 'w') as f:
        f.write("Fixed Room Visualizations\n")
        f.write("=" * 50 + "\n\n")
        f.write("Legend:\n")
        f.write("  . = Open floor      # = Wall          + = Secondary wall\n")
        f.write("  ~ = Water           P = Player spawn  S = Stairs down\n")
        f.write("  ! = Special tile    T = Trap/Item     1-6 = Stairs parts\n")
        f.write("\n" + "=" * 50 + "\n\n")

        for room_data in all_rooms:
            f.write(f"Room {room_data['room_num']} ({room_data['width']}x{room_data['height']})\n")
            f.write("-" * 50 + "\n")
            ascii_map = render_ascii_map(room_data['tiles'], room_data['width'], room_data['height'])
            for line in ascii_map:
                f.write(line + "\n")
            f.write("\n")

    print(f"Generated {summary_file}")

if __name__ == '__main__':
    main()
