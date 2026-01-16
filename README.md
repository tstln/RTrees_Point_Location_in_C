# R-Tree Implementation & Point Location

This project implements an R-Tree spatial index (using a 2D mesh) to perform efficient point-in-triangle searches. It includes a C implementation, Gnuplot visualizations, and a detailed LaTeX report. Please refer to that report in the root directory for more details!

Ressources used : R-Tree implementation provided by Superliminal (http:/superliminal.com/sources/), meshes data and files for handling them from Didier Smets and its course "Algorithm and Data Structures".

## Project Structure

```bash
.
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── plots/                  # Output folder for Gnuplot scripts/images
├── latex/                  # LaTeX report source
├── meshes/                 # Test meshes (.mesh format)
├── src/                    # Source code (main, mesh loading, exporter)
├── include/                # Header files
├── RTree_from_superliminal # Modified R-Tree library from SuperLiminal
└── what_query_point.dat    # Configuration for search query
```

## Prerequisites

- **CMake** (Build system)
- **GCC** (Compiler)
- **Gnuplot** (For generating visualization images)

## Build and Run

### 1. Build the Project
The project uses **CMake**.

```bash
cmake -S . -B build
cmake --build build
```

### 2. Run the Executable
The executable "RTreeRUN" is generated in the "build/" directory.

**Syntax:**
```bash
./build/RTreeRUN <mesh_file> [num_test_points]
```

**Examples:**

*Standard Run (with provided TP2 mesh):*
```bash
./build/RTreeRUN meshes/mesh2-tp2.mesh 1000
```

*Greenland Mesh:*
```bash
./build/RTreeRUN meshes/greenland.mesh 1000
```

## Visualization

The program uses **Gnuplot** to visualize the mesh, the R-Tree structure (levels), and the search results.
After running the program, the plotting scripts are generated in the "plots/" directory.

**To generate the images (PNG):**
```bash
cd plots
gnuplot *.gp
```
This will create files like "visualization_overview.png", "viz_level_0_1.png", etc.

## Configuration

- **Query Point**: You can change the specific point to search for (displayed in green) by editing "what_query_point.dat" in the root directory.
  - Format: "X Y" (e.g., "0.1 0.1")
