#include "../include/GnuplotExporter.h"
#include "../include/RTreeWrapper.h"
#include "../include/mesh_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double GetTime() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <mesh_file> [num_test_points]\n", argv[0]);
    return 1;
  }

  const char *meshFile = argv[1];
  int numPoints = (argc > 2) ? atoi(argv[2]) : 1000;

  printf("Loading mesh %s...\n", meshFile);
  struct Mesh mesh;
  initialize_mesh(&mesh);
  if (read_mesh_from_medit_file(&mesh, meshFile) != 0) {
    printf("Failed to load mesh: %s\n", meshFile);
    return 1;
  }
  printf("Mesh loaded: %d vertices, %d triangles.\n", mesh.nvert, mesh.ntri);

  // Find mesh bbox for visualization and random points
  double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
  for (int i = 0; i < mesh.nvert; i++) {
    if (mesh.vertices[i].x < minX)
      minX = mesh.vertices[i].x;
    if (mesh.vertices[i].x > maxX)
      maxX = mesh.vertices[i].x;
    if (mesh.vertices[i].y < minY)
      minY = mesh.vertices[i].y;
    if (mesh.vertices[i].y > maxY)
      maxY = mesh.vertices[i].y;
  }
  printf("Mesh BBox: [%.2f, %.2f] x [%.2f, %.2f]\n", minX, maxX, minY, maxY);

  printf("Building R-Tree...\n");
  double start = GetTime();
  struct Node *root = BuildRTree(&mesh);
  double end = GetTime();
  printf("R-Tree built in %.6f seconds.\n", end - start);

  printf("Exporting to Gnuplot to 'plots/' directory...\n");
  ExportMeshToGnuplot(&mesh, "plots/mesh_edges.dat");
  int treeHeight = ExportRTreeLevels(root);
  GenerateGnuplotScripts(treeHeight, minX, maxX, minY, maxY);

  // Pick a point INSIDE the mesh to visualize
  struct Vertex query_point = {0.0, 0.0, 0.0};

  FILE *qf = fopen("what_query_point.dat", "r");
  if (qf) {
    double qx, qy;
    if (fscanf(qf, "%lf %lf", &qx, &qy) == 2) {
      query_point.x = qx;
      query_point.y = qy;
      printf("Loaded query point from 'what_query_point.dat': (%.2f, %.2f)\n",
             qx, qy);
    } else {
      printf("WARNING: failed to read coordinates from 'what_query_point.dat', "
             "using default (0,0).\n");
    }
    fclose(qf);
  } else {
    printf("WARNING: 'what_query_point.dat' not found. Using default (0,0).\n");
    query_point.x = 0.0;
    query_point.y = 0.0;
  }

  ExportPointToGnuplot(query_point, "plots/query_point.dat");

  // Find the triangle containing the query point
  int foundTriIndex = FindTriangle(root, &mesh, query_point);
  if (foundTriIndex != -1) {
    ExportTriangleToGnuplot(mesh.triangles[foundTriIndex], &mesh,
                            "plots/found_triangle.dat");
    printf("Exported 'plots/found_triangle.dat'.\n");
  } else {
    printf("WARNING: Query point not found in any triangle!\n");
  }

  printf("Exported 'mesh_edges.dat', 'query_point.dat', 'found_triangle.dat', "
         "and level_* files.\n");
  printf("Generated Gnuplot scripts for levels visualization.\n");

  printf("Generating %d random test points to compare R-Tree vs "
         "Naive search...\n",
         numPoints);

  struct Vertex *test_points =
      (struct Vertex *)malloc(sizeof(struct Vertex) * numPoints);
  srand(time(NULL));
  for (int i = 0; i < numPoints; i++) {
    // For random points, we just need x and y within bbox. z=0.
    struct Vertex p;
    p.x = minX + (maxX - minX) * ((double)rand() / RAND_MAX);
    p.y = minY + (maxY - minY) * ((double)rand() / RAND_MAX);
    p.z = 0.0;
    test_points[i] = p;
  }

  printf("Benchmarking R-Tree Search...\n");
  int hitsRTree = 0;
  start = GetTime();
  for (int i = 0; i < numPoints; i++) {
    if (FindTriangle(root, &mesh, test_points[i]) != -1) {
      hitsRTree++;
    }
  }
  end = GetTime();
  double timeRTree = end - start;
  printf("R-Tree: %.6f seconds (%d hits)\n", timeRTree, hitsRTree);

  printf("Benchmarking Naive Search...\n");
  int hitsNaive = 0;
  start = GetTime();
  for (int i = 0; i < numPoints; i++) {
    struct Vertex current_test_point = test_points[i];
    for (int j = 0; j < mesh.ntri; j++) {
      struct Triangle t = mesh.triangles[j];
      struct Vertex p1 = mesh.vertices[t.v1];
      struct Vertex p2 = mesh.vertices[t.v2];
      struct Vertex p3 = mesh.vertices[t.v3];
      if (IsPointInTriangle(current_test_point, p1, p2, p3)) {
        hitsNaive++;
        break; // Found one
      }
    }
  }
  end = GetTime();
  double timeNaive = end - start;
  printf("Naive:  %.6f seconds (%d hits)\n", timeNaive, hitsNaive);

  printf("Speedup: %.2fx\n", timeNaive / timeRTree);
  printf("Absolute Time Difference: %.6f seconds\n", timeNaive - timeRTree);

  // Correctness check
  if (hitsRTree != hitsNaive) {
    printf("WARNING: Hit counts mismatch! RTree: %d, Naive: %d\n", hitsRTree,
           hitsNaive);
  } else {
    printf("Correctness Check: PASS (Hit counts match)\n");
  }

  free(test_points);
  dispose_mesh(&mesh);
  /* Note: RTreeFreeNode(root) should be needed ?
  Although the library has "RTreeFreeNode", it only frees one node.
  Typically we'd need a recursive free, to deallocate its children nodes (and
  their children nodes, etc). For this short lived program, OS will clean up.
  */

  return 0;
}
