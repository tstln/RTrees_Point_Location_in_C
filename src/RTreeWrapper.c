#include "../include/RTreeWrapper.h"

// To calculate min/max
static double min(double a, double b) { return a < b ? a : b; }
static double max(double a, double b) { return a > b ? a : b; }

// Barycentric coordinate check (2D only, ignores z)
int IsPointInTriangle(struct Vertex p, struct Vertex a, struct Vertex b,
                      struct Vertex c) {
  double v0x = c.x - a.x;
  double v0y = c.y - a.y;
  double v1x = b.x - a.x;
  double v1y = b.y - a.y;
  double v2x = p.x - a.x;
  double v2y = p.y - a.y;

  double dot00 = v0x * v0x + v0y * v0y;
  double dot01 = v0x * v1x + v0y * v1y;
  double dot02 = v0x * v2x + v0y * v2y;
  double dot11 = v1x * v1x + v1y * v1y;
  double dot12 = v1x * v2x + v1y * v2y;

  double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
  double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
  double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

  return (u >= 0) && (v >= 0) && (u + v <= 1);
}

struct Node *BuildRTree(const struct Mesh *mesh) {
  struct Node *root = RTreeNewIndex();

  for (int i = 0; i < mesh->ntri; i++) {
    struct Triangle t = mesh->triangles[i];
    struct Vertex p1 = mesh->vertices[t.v1];
    struct Vertex p2 = mesh->vertices[t.v2];
    struct Vertex p3 = mesh->vertices[t.v3];

    struct Rect rect;
    // Identify min and max for bounding box
    rect.boundary[0] = min(p1.x, min(p2.x, p3.x)); // xmin
    rect.boundary[1] = min(p1.y, min(p2.y, p3.y)); // ymin
    rect.boundary[2] = max(p1.x, max(p2.x, p3.x)); // xmax
    rect.boundary[3] = max(p1.y, max(p2.y, p3.y)); // ymax

    // Insert into RTree. ID must be > 0. using i+1.
    RTreeInsertRect(&rect, i + 1, &root, 0);
  }
  return root;
}

// Callback context
typedef struct {
  const struct Mesh *mesh;
  struct Vertex p;
  int foundIndex;
} SearchContext;

int SearchCallback(int id, void *arg) {
  SearchContext *ctx = (SearchContext *)arg;
  int triIndex = id - 1; // Convert back to 0-based

  struct Triangle t = ctx->mesh->triangles[triIndex];
  struct Vertex p1 = ctx->mesh->vertices[t.v1];
  struct Vertex p2 = ctx->mesh->vertices[t.v2];
  struct Vertex p3 = ctx->mesh->vertices[t.v3];

  if (IsPointInTriangle(ctx->p, p1, p2, p3)) {
    ctx->foundIndex = triIndex;
    return 0; // Stop search
  }
  return 1; // Continue search
}

int FindTriangle(struct Node *root, const struct Mesh *mesh, struct Vertex p) {
  struct Rect searchRect;
  // Degenerate rect (point)
  searchRect.boundary[0] = p.x;
  searchRect.boundary[1] = p.y;
  searchRect.boundary[2] = p.x;
  searchRect.boundary[3] = p.y;

  SearchContext ctx;
  ctx.mesh = mesh;
  ctx.p = p;
  ctx.foundIndex = -1;

  RTreeSearch(root, &searchRect, SearchCallback, &ctx);

  return ctx.foundIndex;
}
