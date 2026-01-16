
#include "CARD.H"
#include "Index.h"
#include "assert.h"
#include <malloc.h>
#include <stdio.h>

// Initialize one branch cell in a node.
//
static void RTreeInitBranch(struct Branch *b) {
  RTreeInitRect(&(b->rect));
  b->child = NULL;
  /* Ed.

  */
}

// Initialize a Node structure.
// ed. no malloc inside, so that we can use an existing node (and "clean it"). N
// need to be mallocated first if new node (then instead use RTreeNewNode).
void RTreeInitNode(struct Node *N) {
  register struct Node *n = N;
  register int i;
  n->count = 0;  // ed. no valid child
  n->level = -1; // ed. undefined level : "waiting to assign a level". 0 is for
                 // leaf and >0 for true nodes
  for (i = 0; i < MAXCARD; i++)
    RTreeInitBranch(&(n->branch[i])); // ed. therefore initializing (eg.
                                      // cleaning) all branches
}

// Make a new node and initialize to have all branch cells empty.
//
struct Node *RTreeNewNode() {
  register struct Node *n;

  // n = new Node;
  n = (struct Node *)malloc(sizeof(struct Node));
  assert(n);
  RTreeInitNode(n);
  return n;
}

void RTreeFreeNode(struct Node *p) {
  assert(p);
  // delete p;
  free(p);
}

static void RTreePrintBranch(struct Branch *b, int depth) {
  RTreePrintRect(&(b->rect), depth);
  RTreePrintNode(b->child, depth);
}

extern void RTreeTabIn(int depth) {
  int i;
  for (i = 0; i < depth; i++)
    putchar('\t');
}

// Print out the data in a node.
//
void RTreePrintNode(struct Node *n, int depth) {
  int i;
  assert(n);

  RTreeTabIn(depth);
  printf("node");
  if (n->level == 0)
    printf(" LEAF");
  else if (n->level > 0)
    printf(" NONLEAF");
  else
    printf(" TYPE=?");
  printf("  level=%d  count=%d  address=%o\n", n->level, n->count, n);

  for (i = 0; i < n->count; i++) {
    if (n->level == 0) {
      // RTreeTabIn(depth);
      // printf("\t%d: data = %d\n", i, n->branch[i].child);

      /* Ed.
      Two lines are commented to not be flood under the listing of all the data
      ! Note : not sure if that works anyway ; may need to cast properly :
      (int)(long long)n->branch[i].child because on a 64bit machine pointers are
      8 bytes and int are 4 bytes (instead of 4 and 4 on 32bit so
      interchangeable) for compability we can cast to long long (to force 8
      bytes both on 32bit and 64bit machines) but then it might be less
      efficient (printing isn't anyways so...) or we could just use size_t ?

      Ed. new : solution found with intptr_t (int pointer type), to test here
      someday
      */
    } else {
      RTreeTabIn(depth);
      printf("branch %d\n", i);
      RTreePrintBranch(&n->branch[i], depth + 1);
    }
  }
}

// Find the smallest rectangle that includes all rectangles in
// branches of a node.
//
struct Rect RTreeNodeCover(struct Node *N) {
  register struct Node *n = N;
  register int i, first_time = 1;
  struct Rect r;
  assert(n);

  RTreeInitRect(&r);
  for (i = 0; i < MAXKIDS(n); i++) // ed. and not MAXCARD
    // ed. intuition : it can have the same value as MAXCARD (see NODECARD and
    // LEAFCARD in card.h,.c), depending of if we are in a leaf or not ed. so
    // that function would still work even if we change the size of the leafs
    // without changing the size of the internal nodes
    if (n->branch[i].child) {
      if (first_time) // ed. first valid rectangle we found (r becomes that
                      // rectangle)
      {
        r = n->branch[i].rect;
        first_time = 0;
      } else // ed. not the first, we need to make r bigger to include that new
             // rectangle
        r = RTreeCombineRect(&r, &(n->branch[i].rect));
    }
  return r;
}

// Pick a branch.  Pick the one that will need the smallest increase
// in area to accomodate the new rectangle.  This will result in the
// least total area for the covering rectangles in the current node.
// In case of a tie, pick the one which was smaller before, to get
// the best resolution when searching.
//
int RTreePickBranch(struct Rect *R, struct Node *N) {
  register struct Rect *r = R; // ed. the rect that we want to insert
  register struct Node *n =
      N; // ed. current node where we have to find a branch to insert R
  register struct Rect *rr;
  register int i, first_time = 1; // ed. first time already at 1 (because R has
                                  // to be a correct rectangle)
  RectReal increase, bestIncr = (RectReal)-1, area, bestArea;
  int best;
  struct Rect tmp_rect;
  assert(r && n);

  for (i = 0; i < MAXKIDS(n);
       i++) // ed. we want to loop on all children (all branches) of the node
  {
    if (n->branch[i].child) // ed. if branch exists
    {
      rr = &n->branch[i].rect;             // ed. "current mbr of the child"
      area = RTreeRectSphericalVolume(rr); // ed. cf. sphvol.c, current area
      tmp_rect = RTreeCombineRect(
          r, rr); // ed. what would be the mbr if we combine it with new rect r
      increase = RTreeRectSphericalVolume(&tmp_rect) -
                 area; // ed. the difference of area between the combination and
                       // the original
      if (increase < bestIncr ||
          first_time) // ed. best candidate that leads to the least increase yet
      {
        best = i;
        bestArea = area;
        bestIncr = increase;
        first_time = 0;
      } else if (increase == bestIncr &&
                 area < bestArea) // ed. special case, if we have the same
                                  // increase (for two branches then) we just
                                  // choose the smallest branch
      {
        best = i;
        bestArea = area;
        bestIncr = increase;
      }
    }
  }
  return best;
}

// Add a branch to a node.  Split the node if necessary.
// Returns 0 if node not split.  Old node updated.
// Returns 1 if node split, sets *new_node to address of new node.
// Old node updated, becomes one of two.
//
int RTreeAddBranch(struct Branch *B, struct Node *N, struct Node **New_node) {
  register struct Branch *b = B;
  register struct Node *n = N;
  register struct Node **new_node = New_node;
  register int i;

  assert(b);
  assert(n);

  if (n->count < MAXKIDS(n)) /* split won't be necessary */
  {
    for (i = 0; i < MAXKIDS(n); i++) /* find empty branch */
    {
      if (n->branch[i].child == NULL) {
        n->branch[i] = *b;
        n->count++;
        break;
      }
    }
    return 0;
  } else {
    assert(new_node);
    RTreeSplitNode(n, b,
                   new_node); // ed. cf. either the quadratic algorithm (in
                              // split_q.c) or the linear (in split_l.c)
    return 1;
  }
}

// Disconnect a dependent node.
//
void RTreeDisconnectBranch(struct Node *n, int i) {
  assert(n && i >= 0 && i < MAXKIDS(n));
  assert(n->branch[i].child);

  RTreeInitBranch(&(n->branch[i]));
  n->count--;
}
