#include "../include/GnuplotExporter.h"
#include <stdio.h>
#include <stdlib.h>

void ExportMeshToGnuplot(const struct Mesh *mesh, const char *filename) {
  FILE *file = fopen(filename, "w");
  if (!file) {
    printf("Error opening %s for writing\n", filename);
    return;
  }

  for (int i = 0; i < mesh->ntri; i++) {
    struct Triangle t = mesh->triangles[i];
    struct Vertex p1 = mesh->vertices[t.v1];
    struct Vertex p2 = mesh->vertices[t.v2];
    struct Vertex p3 = mesh->vertices[t.v3];

    fprintf(file, "%f %f\n", p1.x, p1.y);
    fprintf(file, "%f %f\n", p2.x, p2.y);
    fprintf(file, "%f %f\n", p3.x, p3.y);
    fprintf(file, "%f %f\n", p1.x, p1.y); // Close loop
    fprintf(file, "\n\n");                // Disconnect from next triangle
  }

  fclose(file);
}

// Recursive function to get max depth (height) of the tree
static int GetTreeHeight(struct Node *node) {
  if (!node)
    return 0;
  if (node->level == 0)
    return 1;                                      // Leaf level in library is 0
  return 1 + GetTreeHeight(node->branch[0].child); // R-Tree is balanced
}

// Recursive function to write nodes to specific level files
static void WriteNodesToLevels(struct Node *node, int treeHeight,
                               FILE **files) {
  if (!node)
    return;

  // Library uses level 0 for leaves.
  // We want Root to be Level 0.
  // So current level index = (treeHeight - 1) - node->level
  int displayLevel = (treeHeight - 1) - node->level;

  FILE *f = files[displayLevel];

  for (int i = 0; i < node->count; i++) {
    struct Branch *b = &node->branch[i];
    struct Rect *r = &b->rect;

    fprintf(f, "%f %f\n", r->boundary[0], r->boundary[1]);
    fprintf(f, "%f %f\n", r->boundary[2], r->boundary[1]);
    fprintf(f, "%f %f\n", r->boundary[2], r->boundary[3]);
    fprintf(f, "%f %f\n", r->boundary[0], r->boundary[3]);
    fprintf(f, "%f %f\n", r->boundary[0], r->boundary[1]);
    fprintf(f, "\n\n");

    if (node->level > 0) {
      WriteNodesToLevels(b->child, treeHeight, files);
    }
  }
}

int ExportRTreeLevels(struct Node *root) {
  if (!root)
    return 0;

  int height = GetTreeHeight(root);
  printf("DEBUG: Tree Height is %d\n", height);

  // Open file pointers for each level
  FILE **files = (FILE **)malloc(sizeof(FILE *) * height);
  for (int i = 0; i < height; i++) {
    char filename[32];
    sprintf(filename, "plots/level_%d.dat", i);
    files[i] = fopen(filename, "w");
    if (!files[i]) {
      printf("Error opening %s\n", filename);
    }
  }

  WriteNodesToLevels(root, height, files);

  for (int i = 0; i < height; i++) {
    if (files[i])
      fclose(files[i]);
  }
  free(files);

  return height;
}

void GenerateGnuplotScripts(int height, double minX, double maxX, double minY,
                            double maxY) {
  // Add a 5% margin
  double width = maxX - minX;
  double heightBox = maxY - minY;
  double xMargin = width * 0.05;
  double yMargin = heightBox * 0.05;

  double x1 = minX - xMargin;
  double x2 = maxX + xMargin;
  double y1 = minY - yMargin;
  double y2 = maxY + yMargin;

  // Determine image dimensions based on aspect ratio
  // Max dimension 1024 to ensure good resolution
  int maxDim = 1024;
  int imgW, imgH;
  if (width >= heightBox) {
    imgW = maxDim;
    imgH = (int)(maxDim * (heightBox / width));
  } else {
    imgH = maxDim;
    imgW = (int)(maxDim * (width / heightBox));
  }
  // Ensure minimum dimensions
  if (imgW < 400)
    imgW = 400;
  if (imgH < 400)
    imgH = 400;

  // 1. Generate viz_all_levels.gp
  FILE *f = fopen("plots/viz_all_levels.gp", "w");
  if (f) {
    fprintf(f, "set terminal pngcairo size 800,800\n");
    fprintf(f, "set output 'visualization_levels_legend.png'\n");
    fprintf(f, "set title \"R-Tree Levels (Root=0)\"\n");
    fprintf(f, "set xrange [%f:%f]\n", x1, x2);
    fprintf(f, "set yrange [%f:%f]\n", y1, y2);
    fprintf(f, "set size square\n");
    fprintf(f, "set key outside\n\n");
    fprintf(f, "p \"mesh_edges.dat\" w l lc rgb \"#CCCCCC\" title \"Mesh\"");

    const char *colors[] = {"red",    "blue", "green", "orange",
                            "purple", "cyan", "brown"};
    int numColors = 7;

    for (int i = 0; i < height; i++) {
      fprintf(
          f,
          ", \\\n  \"level_%d.dat\" w l lw 2 lc rgb \"%s\" title \"Level %d\"",
          i, colors[i % numColors], i);
    }
    fprintf(f, "\n");
    fclose(f);
    printf("Generated 'viz_all_levels.gp'\n");
  }

  // 2. Generate consecutive level scripts
  for (int i = 0; i < height - 1; i++) {
    char filename[64];
    sprintf(filename, "plots/viz_level_%d_%d.gp", i, i + 1);
    f = fopen(filename, "w");
    if (f) {
      fprintf(f, "set terminal pngcairo size 800,800\n");
      fprintf(f, "set output 'viz_level_%d_%d.png'\n", i, i + 1);
      fprintf(f, "set title \"R-Tree Levels %d and %d\"\n", i, i + 1);
      fprintf(f, "set xrange [%f:%f]\n", x1, x2);
      fprintf(f, "set yrange [%f:%f]\n", y1, y2);
      fprintf(f, "set size square\n");
      fprintf(f, "set key top right\n\n");
      fprintf(
          f,
          "p \"mesh_edges.dat\" w l lc rgb \"#EEEEEE\" title \"Mesh\", \\\n");
      fprintf(f,
              "  \"level_%d.dat\" w l lw 2 lc rgb \"red\" title \"Level %d "
              "(Parent)\", \\\n",
              i, i);
      fprintf(f,
              "  \"level_%d.dat\" w l lw 2 lc rgb \"blue\" title \"Level %d "
              "(Child)\"\n",
              i + 1, i + 1);
      fclose(f);
      printf("Generated '%s'\n", filename);
    }
  }

  // 3. Generate viz_overview.gp (All levels in light red + Highlight)
  f = fopen("plots/viz_overview.gp", "w");
  if (f) {
    fprintf(f, "set terminal pngcairo size 800,800\n");
    fprintf(f, "set output 'visualization_overview.png'\n");
    fprintf(f, "set title \"R-Tree Structure (All Nodes)\"\n");
    fprintf(f, "set xrange [%f:%f]\n", x1, x2);
    fprintf(f, "set yrange [%f:%f]\n", y1, y2);
    fprintf(f, "set size square\n");
    fprintf(f, "set key outside\n\n");

    fprintf(f, "p \"mesh_edges.dat\" w l lc rgb \"blue\" title \"Mesh\"");

    // Plot all levels in light red
    for (int i = 0; i < height; i++) {
      if (i == 0) {
        fprintf(f,
                ", \\\n  \"level_%d.dat\" w l lw 1 lc rgb \"#FF8888\" title "
                "\"R-Tree Nodes\"",
                i);
      } else {
        fprintf(f,
                ", \\\n  \"level_%d.dat\" w l lw 1 lc rgb \"#FF8888\" notitle",
                i);
      }
    }

    // Plot highlights on top
    fprintf(f, ", \\\n  \"found_triangle.dat\" w l lw 3 lc rgb \"magenta\" "
               "title \"Found Triangle\"");
    fprintf(f, ", \\\n  \"query_point.dat\" w p pt 7 ps 2 lc rgb \"green\" "
               "title \"Query Point\"\n");

    fclose(f);
    printf("Generated 'viz_overview.gp'\n");
  }
}

void ExportPointToGnuplot(struct Vertex p, const char *filename) {
  FILE *file = fopen(filename, "w");
  if (!file) {
    printf("Error opening %s for writing\n", filename);
    return;
  }
  fprintf(file, "%f %f\n", p.x, p.y);
  fclose(file);
}

void ExportTriangleToGnuplot(struct Triangle t, const struct Mesh *mesh,
                             const char *filename) {
  FILE *file = fopen(filename, "w");
  if (!file) {
    printf("Error opening %s for writing\n", filename);
    return;
  }
  struct Vertex p1 = mesh->vertices[t.v1];
  struct Vertex p2 = mesh->vertices[t.v2];
  struct Vertex p3 = mesh->vertices[t.v3];

  fprintf(file, "%f %f\n", p1.x, p1.y);
  fprintf(file, "%f %f\n", p2.x, p2.y);
  fprintf(file, "%f %f\n", p3.x, p3.y);
  fprintf(file, "%f %f\n", p1.x, p1.y); // Close loop
  fclose(file);
}
