#ifndef RTREEWRAPPER_H
#define RTREEWRAPPER_H

#include "../RTree_from_superliminal/Index.h" // Function prototypes from 'RTree_from_superliminal'
#include "mesh.h"

// Builds an R-Tree from the given mesh.
// Returns the root node of the R-Tree.
struct Node *BuildRTree(const struct Mesh *mesh);

// Finds the index of the triangle containing point p.
// Returns triangle index or -1 if not found.
int FindTriangle(struct Node *root, const struct Mesh *mesh, struct Vertex p);

int IsPointInTriangle(struct Vertex p, struct Vertex a, struct Vertex b,
                      struct Vertex c);

#endif
