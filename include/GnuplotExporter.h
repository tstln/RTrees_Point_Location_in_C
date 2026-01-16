#ifndef GNUPLOTEXPORTER_H
#define GNUPLOTEXPORTER_H

#include "../RTree_from_superliminal/Index.h"
#include "mesh.h"

// Exports mesh edges to a data file for Gnuplot
void ExportMeshToGnuplot(const struct Mesh *mesh, const char *filename);

// Exports R-Tree nodes split by level (Root=0) to level_X.dat files
// Also returns the height of the tree
int ExportRTreeLevels(struct Node *root);

// Generates Gnuplot scripts for levels and overview
void GenerateGnuplotScripts(int height, double minX, double maxX, double minY,
                            double maxY);

// Exports a single point to a data file for Gnuplot
void ExportPointToGnuplot(struct Vertex p, const char *filename);

// Exports a single triangle to a data file for Gnuplot
void ExportTriangleToGnuplot(struct Triangle t, const struct Mesh *mesh,
                             const char *filename);

#endif
